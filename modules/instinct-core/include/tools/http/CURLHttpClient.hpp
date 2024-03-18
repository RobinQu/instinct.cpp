//
// Created by vscode on 3/18/24.
//

#ifndef INSTINCT_CURLHTTPCLIENT_HPP
#define INSTINCT_CURLHTTPCLIENT_HPP

#include <curl/curl.h>


#include "CoreGlobals.hpp"
#include "IHttpClient.hpp"
#include "HttpUtils.hpp"

namespace INSTINCT_CORE_NS {

    namespace details {
//
//        struct curl_session_data {
//            HttpRequest* request;
//            HttpResponse* response;
//        };


        static void make_curl_headers(const HttpHeaders& headers, curl_slist **header_slist) {
            for(const auto& [k,v]: headers) {
                *header_slist = curl_slist_append(*header_slist, fmt::format("{}: {}", k, v).data());
            }
        }

        static size_t curl_header_callback(char *ptr, size_t size, size_t nmemb,
                               void *http_response) {
            auto response = static_cast<HttpResponse*>(http_response);


        }

        static size_t curl_write_callback(char *ptr, size_t size, size_t nmemb,
                              void *http_response) {

        }

        static int make_post_request(
            const HttpRequest &request,
            HttpResponse& response
        ) {
            CURLcode ret;
            CURL *hnd;
            struct curl_slist *header_slist = nullptr;
            make_curl_headers(request.headers, &header_slist);

            hnd = curl_easy_init();
            curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);

            auto url_string = HttpUtils::CreateUrlString(request);
            curl_easy_setopt(hnd, CURLOPT_URL, url_string.c_str());
            curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request.body.data());
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)request.body.size());
            curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, header_slist);
            curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.4.0");
            curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 5L);
            curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
            curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
            curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

            curl_easy_setopt(hnd, CURLOPT_HEADER, 1L);
//            curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, curl_header_callback);
//            curl_easy_setopt(hnd, CURLOPT_HEADERDATA, &response);

            { // dump headers to response
                struct curl_header *h;
                struct curl_header *prev = nullptr;
                do {
                    h = curl_easy_nextheader(hnd, CURLH_HEADER, -1, prev);
                    if(h) {
                        printf(" %s: %s (%u)\n", h->name, h->value, (int)h->amount);
                        response.headers[h->name] = h->value;
                    }

                    prev = h;
                } while(h);
            }

            curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_write_callback);
            curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

            ret = curl_easy_perform(hnd);
            curl_easy_cleanup(hnd);
            hnd = nullptr;
            curl_slist_free_all(header_slist);
            header_slist = nullptr;
            return ret;
        }
    }

    class CURLHttpClient final: public IHttpClient {

    public:
        HttpResponse Execute(const HttpRequest &call) override {

        }

        AsyncIterator<std::string> StreamChunk(const HttpRequest &call) override {

        }


    };
}

#endif //INSTINCT_CURLHTTPCLIENT_HPP
