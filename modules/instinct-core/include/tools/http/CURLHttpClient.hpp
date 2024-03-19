//
// Created by vscode on 3/18/24.
//

#ifndef INSTINCT_CURLHTTPCLIENT_HPP
#define INSTINCT_CURLHTTPCLIENT_HPP

#include <curl/curl.h>


#include "CoreGlobals.hpp"
#include "IHttpClient.hpp"
#include "HttpUtils.hpp"
#include "HttpClientException.hpp"

namespace INSTINCT_CORE_NS {

    namespace details {

        static void initialize_curl() {
            static bool CURL_INITIALIZED = false;
            if(!CURL_INITIALIZED) {
                curl_global_init(CURL_GLOBAL_ALL);
                CURL_INITIALIZED = true;
            }
        }


        static size_t curl_write_callback(char *ptr, size_t size, size_t nmemb,
                                          HttpResponse *http_response) {
            http_response->body += {ptr, size*nmemb};
            return size * nmemb;
        }

        static void configure_curl_request(
                const HttpRequest &request,
                CURL *hnd,
                curl_slist *header_slist
                ) {

            for(const auto& [k,v]: request.headers) {
                header_slist = curl_slist_append(header_slist, fmt::format("{}: {}", k, v).data());
            }
            curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);

            auto url_string = HttpUtils::CreateUrlString(request);
            curl_easy_setopt(hnd, CURLOPT_URL, url_string.c_str());
            curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);

            if (!request.body.empty() && (request.method == HttpMethod::kPOST || request.method == kPUT)) {
                curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request.body.data());
                curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)request.body.size());

            }
//            if (request.method == kPUT) {
//                curl_easy_setopt(hnd, CURLOPT_UPLOAD, 1L);
//            }

            curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, header_slist);
            curl_easy_setopt(hnd, CURLOPT_ACCEPT_ENCODING, "");
            curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.4.0");
            curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 5L);
            curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
            curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, fmt::format("{}", request.method).c_str());
            curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
            curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
        }

        static CURLcode make_curl_request(
            const HttpRequest &request,
            HttpResponse& response
        ) {
            initialize_curl();
            CURLcode ret;
            CURL *hnd;
            hnd = curl_easy_init();
            struct curl_slist *header_slist = nullptr;

            configure_curl_request(request, hnd, header_slist);

            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_callback);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

            ret = curl_easy_perform(hnd);

            if(ret==0) {
                // get response code
                curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &response.status_code);
                { // dump headers to response
                    struct curl_header *h;
                    struct curl_header *prev = nullptr;
                    do {
                        h = curl_easy_nextheader(hnd, CURLH_HEADER, -1, prev);
                        if(h) {
//                            printf(" %s: %s (%u)\n", h->name, h->value, (int)h->amount);
                            response.headers[h->name] = h->value;
                        }

                        prev = h;
                    } while(h);
                }
            }

            curl_easy_cleanup(hnd);
            curl_slist_free_all(header_slist);
            return ret;
        }


        template<typename OB>
        requires rpp::constraint::observer_of_type<OB, std::string>
        static size_t curl_write_callback_with_observer(char *ptr, size_t size, size_t nmemb,
                                                        OB *ob) {
            std::string buf = {ptr, size * nmemb};
            ob->on_next(buf);
            return size * nmemb;
        }

        template<typename OB>
        requires rpp::constraint::observer_of_type<OB, std::string>
        static CURLcode observe_curl_request(const HttpRequest &request, OB&& observer) {
            initialize_curl();
            CURLcode ret;
            CURL *hnd;
            struct curl_slist *header_slist = nullptr;
            hnd = curl_easy_init();

            configure_curl_request(request, hnd, header_slist);
            using OB_TYPE = std::decay_t<OB>;
            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_callback_with_observer<OB_TYPE>);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &observer);

            ret = curl_easy_perform(hnd);
            if(ret==0) {
                int status_code;
                curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &status_code);
                if (status_code >= 400) {
                    observer.on_error(std::make_exception_ptr(HttpClientException(status_code, "Failed to get chunked response")));
                }
            }
            curl_easy_cleanup(hnd);
            curl_slist_free_all(header_slist);
            return ret;
        }
    }

    class CURLHttpClient final: public IHttpClient {

    public:
        HttpResponse Execute(const HttpRequest &call) override {
            HttpResponse http_response;
            auto url = HttpUtils::CreateUrlString(call);
            LOG_DEBUG("REQ: {} {}", call.method, url);
            auto code = details::make_curl_request(call, http_response);
            if (code != 0) {
                throw HttpClientException(-1, "curl request failed with return code " + std::string(curl_easy_strerror(code)));
            }
            LOG_DEBUG("RESP: {} {}, status_code={}, body_length={}", call.method, url, http_response.status_code, http_response.body.size());
            return http_response;
        }

        /**
         *
         * @param call `call` reference should persist during HTTP request
         * @return
         */
        AsyncIterator<std::string> StreamChunk(const HttpRequest &call) override {
            return rpp::source::create<std::string>([&](auto&& observer) {
                using OB_TYPE = decltype(observer);
                auto code = details::observe_curl_request<OB_TYPE>(call, std::forward<OB_TYPE>(observer));
                if (code!=0) {
                    observer.on_error(std::make_exception_ptr(HttpClientException(-1, "curl request failed with return code " + std::string(curl_easy_strerror(code)))));
                }
            });
        }


    };
}

#endif //INSTINCT_CURLHTTPCLIENT_HPP
