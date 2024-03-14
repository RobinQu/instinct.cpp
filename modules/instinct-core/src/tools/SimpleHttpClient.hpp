//
// Created by RobinQu on 2024/1/14.
//

#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include "ChronoUtils.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <httplib.h>
#include <rpp/rpp.hpp>


namespace
INSTINCT_CORE_NS {
    using namespace httplib;

    struct HttpClientOptions {
        unsigned int timeout = 6;
    };

    static std::string string_idenity_fn(std::string v) {
        return v;
    }

    static HttpHeaders flatten_multi_map_headers(const Headers& headers) {
        HttpHeaders http_headers;
        for (const auto& [k,v]: headers) {
            http_headers[k] = v;
        }
        return http_headers;
    }

    class SimpleHttpClient {
        HttpClientOptions options_;
        Client client_;

    public:
        SimpleHttpClient() = delete;

        explicit SimpleHttpClient(const Endpoint& endpoint);

        SimpleHttpClient(const Endpoint& endpoint, const HttpClientOptions& options);

        HttpResponse Execute(
            const HttpRequest& call
        );

        auto StreamChunk(
            const HttpRequest& call
        );
    };

    inline SimpleHttpClient::SimpleHttpClient(const Endpoint& endpoint): SimpleHttpClient(endpoint, {}) {
    }

    inline SimpleHttpClient::SimpleHttpClient(
        const Endpoint& endpoint,
        const HttpClientOptions& options): options_(options),
                                           client_(HttpUtils::CreateConnectionString(endpoint)) {
        assert_positive(endpoint.port, "port number should be a positive integer");
        assert_true(!StringUtils::IsBlankString(endpoint.host), "host cannnot be blank");
    }


    inline HttpResponse SimpleHttpClient::Execute(const HttpRequest& call) {
        Headers headers;
        for (const auto& [k,v]: call.headers) {
            headers.emplace(k, v);
        }

        auto content_type = HttpUtils::GetHeaderValue("content-type", "application/json", call.headers);

        LOG_DEBUG("REQ: method={} path={} body={}", call.method, call.target, call.body);

        HttpResponse response;
        auto t1 = ChronoUtils::GetCurrentTimeMillis();
        switch (call.method) {
            case kUnspecifiedHttpMethod: {
                throw InstinctException("Unspecified http method");
            }

            case kGET: {
                auto get_res = client_.Get(call.target, headers);
                response.body = get_res->body;
                response.headers = flatten_multi_map_headers(get_res->headers);
                response.status_code = get_res->status;
                break;
            }

            case kPOST: {
                auto post_res = client_.Post(call.target, headers, call.body, content_type);
                response.body = post_res->body;
                response.headers = flatten_multi_map_headers(post_res->headers);
                response.status_code = post_res->status;
                break;
            }

            case kPUT: {
                auto put_res = client_.Put(call.target, headers, call.body, content_type);
                response.body = put_res->body;
                response.headers = flatten_multi_map_headers(put_res->headers);
                response.status_code = put_res->status;
                break;
            }


            case kDELETE: {
                auto delete_res = client_.Delete(call.target, headers);
                response.body = delete_res->body;
                response.headers = flatten_multi_map_headers(delete_res->headers);
                response.status_code = delete_res->status;
                break;
            }

            case kHEAD: {
                auto head_res = client_.Head(call.target, headers);
                response.body = head_res->body;
                response.headers = flatten_multi_map_headers(head_res->headers);
                response.status_code = head_res->status;
                break;
            }
        }

        LOG_DEBUG("RESP: method={} path={} rt={} status_code={}, body={}",
                  call.method,
                  call.target,
                  ChronoUtils::GetCurrentTimeMillis() - t1,
                  response.status_code,
                  response.body);
        return response;
    }

    inline auto SimpleHttpClient::StreamChunk(
        const HttpRequest& call
    ) {
        // `call` has to be copied, as downstream may be executed in async way.
        return rpp::source::create<std::string>([&, call](const auto& observer) {
            Request req;
            for (const auto& [k,v]: call.headers) {
                req.set_header(k, v);
            }
            // TODO handle path params
            req.path = call.target;
            req.body = call.body;
            req.method = fmt::format("{}", call.method);
            req.content_receiver = [&](
                const char* data,
                size_t data_length,
                uint64_t offset,
                uint64_t total_length
            ) {
                        observer.on_next(std::string(data, data_length));
                        return true;
                    };
            auto result = client_.send(req);
            if(result->status >= 400) {
                observer.on_error(std::make_exception_ptr(InstinctException("Failed to get chunked response failed with status code: " + std::to_string(result->status))));
            } else {
                observer.on_completed();
            }
        }).as_dynamic();
    }
} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
