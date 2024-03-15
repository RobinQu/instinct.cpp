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

    static HttpResponse conv_to_http_response(const httplib::Result& http_lib_result) {
        if (http_lib_result) {
            HttpResponse response;
            response.body = http_lib_result->body;
            response.headers = flatten_multi_map_headers(http_lib_result->headers);
            response.status_code = http_lib_result->status;
            return response;
        }
        throw InstinctException("Failed HTTP request. Reason: " + to_string(http_lib_result.error()));
    }

    static const std::string LF_LF_END_OF_LINE = "\n\n";

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

        auto StremaServerSentEvent(const HttpRequest& call);
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

        std::string requested_url = fmt::format("{}:{}{}", client_.host(), client_.port(), call.target);
        LOG_DEBUG("{} {} : {}", call.method, requested_url, call.body);

        HttpResponse response;
        auto t1 = ChronoUtils::GetCurrentTimeMillis();
        switch (call.method) {
            case kUnspecifiedHttpMethod: {
                throw InstinctException("Unspecified http method");
            }

            case kGET: {
                auto get_res = client_.Get(call.target, headers);
                response = conv_to_http_response(get_res);
                break;
            }

            case kPOST: {
                auto post_res = client_.Post(call.target, headers, call.body, content_type);
                response = conv_to_http_response(post_res);
                break;
            }

            case kPUT: {
                auto put_res = client_.Put(call.target, headers, call.body, content_type);
                response = conv_to_http_response(put_res);
                break;
            }


            case kDELETE: {
                auto delete_res = client_.Delete(call.target, headers);
                response = conv_to_http_response(delete_res);
                break;
            }

            case kHEAD: {
                auto head_res = client_.Head(call.target, headers);
                response = conv_to_http_response(head_res);
                break;
            }
        }

        LOG_DEBUG("RESP: {} {}, rt={} status_code={}, body={}",
                  call.method,
                  requested_url,
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
                        if (const std::string original_chunk = std::string(data, data_length); original_chunk.find(
                            LF_LF_END_OF_LINE)) {
                            // non-standard line-breaker, used by many SSE impmementaion.
                            for (const auto line: std::views::split(original_chunk, LF_LF_END_OF_LINE)) {
                                auto part = std::string (line.begin(), line.end());
                                if (!part.empty()) {
                                    observer.on_next(part);
                                }
                            }
                        } else {
                            observer.on_next(original_chunk);
                        }
                        return true;
                    };
            auto result = client_.send(req);
            if (result->status >= 400) {
                observer.on_error(std::make_exception_ptr(
                        InstinctException(
                            fmt::format(
                                "Failed to get chunked response. Status code: {}, reason: {}",
                                result->status,
                                result->body
                            )
                        )
                    )
                );
            } else {
                observer.on_completed();
            }
        }).as_dynamic();
    }

    //
    // inline auto SimpleHttpClient::StremaServerSentEvent(const HttpRequest& call) {
    //     Request req;
    //     req.path = call.target;
    //     req.method = fmt::format("{}", call.method);
    //
    // }
} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
