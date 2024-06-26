//
// Created by RobinQu on 3/18/24.
//

#ifndef INSTINCT_CURLHTTPCLIENT_HPP
#define INSTINCT_CURLHTTPCLIENT_HPP

#include <curl/curl.h>
#include <BS_thread_pool.hpp>
#include <csignal>


#include <instinct/core_global.hpp>
#include <instinct/tools/http/http_client.hpp>
#include <instinct/tools/http/http_utils.hpp>
#include <instinct/tools/http/http_client_exception.hpp>
#include <instinct/tools/system_utils.hpp>

namespace INSTINCT_CORE_NS {

    namespace details {

        static BS::thread_pool shared_http_client_thread_pool(
        SystemUtils::GetUnsignedIntEnv("SHARED_HTTP_CLIENT_THREADPOOL_COUNT",
            std::thread::hardware_concurrency())
        );

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
                curl_slist **header_slist
                ) {

            for(const auto& [k,v]: request.headers) {
                *header_slist = curl_slist_append(*header_slist, fmt::format("{}: {}", k, v).data());
            }
            curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);

            auto url_string = HttpUtils::CreateUrlString(request);
            curl_easy_setopt(hnd, CURLOPT_URL, url_string.c_str());
            curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);

            if (!request.body.empty() && (request.method == HttpMethod::kPOST || request.method == kPUT)) {
                curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request.body.data());
                curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)request.body.size());

            }
            curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, *header_slist);
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
            CURL *hnd = curl_easy_init();
            curl_slist *header_slist = nullptr;

            configure_curl_request(request, hnd, &header_slist);

            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_callback);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

            const CURLcode ret = curl_easy_perform(hnd);

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
        struct StreamBuffer {
            OB& ob;
            std::string line_breaker;
            std::string data;
        };

        template<typename OB>
        requires rpp::constraint::observer_of_type<OB, std::string>
        static size_t curl_write_callback_with_observer(char *ptr, size_t size, size_t nmemb, StreamBuffer<OB> *buf) {
            const std::string original_chunk = {ptr, size * nmemb};
            buf->data += original_chunk;
            while(true) {
                if (const auto idx = buf->data.find(buf->line_breaker); idx!=std::string::npos) {
                    buf->ob.on_next(buf->data.substr(0,idx));
                    buf->data = buf->data.substr(idx+buf->line_breaker.size());
                } else {
                    break;
                }
            }
            return size * nmemb;
        }

        template<typename OB>
        requires rpp::constraint::observer_of_type<OB, std::string>
        static CURLcode observe_curl_request(const HttpRequest &request, OB&& observer, const StreamChunkOptions& options) {
            initialize_curl();
            curl_slist *header_slist = nullptr;
            CURL *hnd = curl_easy_init();

            configure_curl_request(request, hnd, &header_slist);
            using OB_TYPE = std::decay_t<OB>;
            StreamBuffer<OB> buf {observer, options.line_breaker};
            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_callback_with_observer<OB_TYPE>);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &buf);

            const CURLcode ret = curl_easy_perform(hnd);
            if(ret==0) {
                // emit remaining data in case the server returns malformed response so that write-callback cannot handle last parts
                if (StringUtils::IsNotBlankString(buf.data)) {
                    observer.on_next(buf.data);
                }
                int status_code = 0;
                curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &status_code);
                if (status_code >= 400) {
                    observer.on_error(std::make_exception_ptr(HttpClientException(status_code, "Failed to get chunked response")));
                } else {
                    observer.on_completed();
                }
            }
            curl_easy_cleanup(hnd);
            curl_slist_free_all(header_slist);
            return ret;
        }


        static size_t curl_write_stream_callback(char *ptr, size_t size, size_t nmemb,
                                          HttpResponseCallback *callback) {
            size *= nmemb;
            return (*callback)({ptr, size}) ? size : 0;
        }


        static CURLcode make_curl_request_with_callback(const HttpRequest& request, HttpStreamResponse& response, const HttpResponseCallback& callback) {
            initialize_curl();
            CURL *hnd = curl_easy_init();
            curl_slist *header_slist = nullptr;

            configure_curl_request(request, hnd, &header_slist);

            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_stream_callback);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &callback);

            const CURLcode ret = curl_easy_perform(hnd);

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

    }



    class CURLHttpClient final: public IHttpClient {

    public:



        HttpResponse Execute(const HttpRequest &call) override {
            HttpResponse http_response;
            auto url = HttpUtils::CreateUrlString(call);
            LOG_DEBUG("REQ: {} {}", call.method, url);
            if (const auto code = details::make_curl_request(call, http_response); code != 0) {
                throw HttpClientException(0, "curl request failed with return code " + std::string(curl_easy_strerror(code)));
            }

            LOG_DEBUG("RESP: {} {}, status_code={}, body_length={}", call.method, url, http_response.status_code, http_response.body.size());
            return http_response;
        }

        /**
         *
         * @param call `call` reference should persist during HTTP request
         * @param options
         * @return
         */
        AsyncIterator<std::string> StreamChunk(const HttpRequest &call, const StreamChunkOptions& options) override {
            assert_true(!options.line_breaker.empty(), "should assign line-breaker");
            HttpUtils::AssertHttpRequest(call);
            // TODO maybe stop copying `call` by using smart pointer
            auto url = HttpUtils::CreateUrlString(call);
            LOG_DEBUG("REQ: {} {}", call.method, url);
            return rpp::source::create<std::string>([&, call, options](auto&& observer) {
                using OB_TYPE = decltype(observer);
                auto code = details::observe_curl_request<OB_TYPE>(call, std::forward<OB_TYPE>(observer), options);
                if (code!=0) {
                    observer.on_error(std::make_exception_ptr(InstinctException("curl request failed with reason: " + std::string(curl_easy_strerror(code)))));
                }
            }) | rpp::ops::tap({}, {}, [&,call]() {
                LOG_DEBUG("RESP: {} {}", call.method, url);
            });
        }

        Futures<HttpResponse> ExecuteBatch(
            const std::vector<HttpRequest>& calls,
            ThreadPool& pool) override {
            const u_int64_t n = calls.size();
            return pool.submit_sequence(u_int64_t{0}, n, [&,calls](auto i) {
                LOG_DEBUG("Executing {} of {} requests", i+1, calls.size());
                return this->Execute(calls[i]);
            });
        }

        HttpStreamResponse ExecuteWithCallback(const HttpRequest &call, const HttpResponseCallback& callback) override {
            HttpStreamResponse http_stream_response {
                {},
                0
            };
            auto url = HttpUtils::CreateUrlString(call);
            if (const auto code = details::make_curl_request_with_callback(call, http_stream_response, callback); code != 0) {
                throw HttpClientException(-1, "curl request failed with return code " + std::string(curl_easy_strerror(code)));
            }
            LOG_DEBUG("RESP: {} {}, status_code={}", call.method, url, http_stream_response.status_code);
            return http_stream_response;
        }
    };

    static HttpClientPtr CreateCURLHttpClient() {
        return std::make_shared<CURLHttpClient>();
    }
}

#endif //INSTINCT_CURLHTTPCLIENT_HPP
