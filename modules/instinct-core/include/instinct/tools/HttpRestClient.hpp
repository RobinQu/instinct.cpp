//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H

#include <utility>

#include <instinct/CoreGlobals.hpp>
#include <instinct/functional/ReactiveFunctions.hpp>

#include <instinct/tools/http/HttpClientException.hpp>
#include <instinct/tools/http/IHttpClient.hpp>
#include <instinct/tools/http/CURLHttpClient.hpp>
#include <instinct/tools/ProtobufUtils.hpp>



namespace INSTINCT_CORE_NS {

    struct ProtobufHttpEntityConverter {
        template<typename T>
        requires IsProtobufMessage<T>
        T Deserialize(const std::string& buf) {
            return ProtobufUtils::Deserialize<T>(buf);
        }

        template<typename T>
        requires IsProtobufMessage<T>
        std::string Serialize(const T& obj) {
            return ProtobufUtils::Serialize(obj);
        }
    };

    namespace details {
        static bool is_end_sentinels(const std::string& chunk_string, const std::vector<std::string>& end_sentinels) {
            return std::ranges::any_of(end_sentinels.begin(), end_sentinels.end(), [&](const auto&item) {
                return item == chunk_string;
            });
        }

        static std::regex DATA_EVENT_PREFIX {"data:\\s+"};
        static std::string strip_data_stream_prefix(const std::string& data_event) {
            return std::regex_replace(data_event, DATA_EVENT_PREFIX, "");
        }


        template<typename RequestEntity, typename ResponseEntity>
        class PostBatchBuilder {
            Endpoint& endpoint;
            HttpClientPtr client;
            ProtobufHttpEntityConverter& converter;
            std::vector<HttpRequest> calls;

        public:
            PostBatchBuilder(Endpoint &endpoint, HttpClientPtr client, ProtobufHttpEntityConverter &converter)
                : endpoint(endpoint),
                  client(std::move(client)),
                  converter(converter) {
            }

            PostBatchBuilder* Add(const std::string& uri, const RequestEntity& param) {
                calls.push_back({
                    endpoint,
                    kPOST,
                    uri,
                    {
                        {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
                    },
                    converter.Serialize(param)
                });
                return this;
            }

            Futures<ResponseEntity> Execute(ThreadPool& pool = shared_http_client_thread_pool) {
                Futures<ResponseEntity> responses;
                for (auto&& http_response_future: client->ExecuteBatch(calls, pool)) {
                    responses.push_back(std::async(std::launch::async, [&, shared_future = std::shared_future(std::move(http_response_future))]() {
                        const auto&[headers, body, status_code] = shared_future.get();
                        if (status_code >= 400) {
                            throw HttpClientException("Error resposne during batched requests", status_code, body);
                        }
                        return converter.Deserialize<ResponseEntity>(body);
                    }));
                }
                return responses;
            }
        };

    }

    class HttpRestClient final {
        ProtobufHttpEntityConverter converter_;
        Endpoint endpoint_;
        HttpClientPtr http_client_;
        HttpHeaders default_headers_;

    public:
        HttpRestClient() = delete;

        explicit HttpRestClient(Endpoint endpoint)
            : endpoint_(std::move(endpoint)) {
            http_client_ = CreateCURLHttpClient();
        }

        HttpHeaders& GetDefaultHeaders() {
            return default_headers_;
        }

        template<typename ResponseEntity>
        ResponseEntity GetObject(const std::string& uri) {
            HttpHeaders headers = default_headers_;
            headers.emplace(HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON));
            const HttpRequest request = {
                    endpoint_,
                    kGET,
                    uri,
                    headers
            };
            const HttpResponse response = http_client_->Execute(request);
            if(response.status_code >= 400) {
                throw HttpClientException(response.status_code, response.body);
            }
            return converter_.Deserialize<ResponseEntity>(response.body);
        }

        template<typename RequestEntity, typename ResponseEntity>
        ResponseEntity PostObject(const std::string& uri, const RequestEntity& param) {
            const std::string param_string = converter_.Serialize(param);
            HttpHeaders headers = default_headers_;
            headers.emplace(HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON));
            const HttpRequest request = {
                    endpoint_,
                    kPOST,
                    uri,
                    headers,
                param_string
            };
            const auto [_, body, status_code] = http_client_->Execute(request);
            if(status_code >= 400) {
                LOG_DEBUG("Non-200 response: {}", body);
                throw HttpClientException(status_code, body);
            }
            return converter_.Deserialize<ResponseEntity>(body);
        }

        template<typename RequestEntity, typename ResponseEntity>
        std::shared_ptr<details::PostBatchBuilder<RequestEntity, ResponseEntity>> CreatePostBatch() {
            return std::make_shared<details::PostBatchBuilder<RequestEntity, ResponseEntity>>(
                endpoint_,
                http_client_,
                converter_
            );
        }

        template<typename RequestEntity, typename ResponseEntity>
        AsyncIterator<ResponseEntity> StreamChunkObject(
            const std::string& uri,
            const RequestEntity& param,
            const bool is_sse_event_stream,
            const std::string& line_breaker,
            const std::vector<std::string>& end_sentinels = {}
        ) {
            HttpHeaders headers = default_headers_;
            headers.emplace(HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON));
            const std::string param_string = converter_.Serialize(param);
            const HttpRequest request = {
                .endpoint = endpoint_,
                .method = kPOST,
                .target = uri,
                .headers = headers,
                .body = param_string
            };

            if (is_sse_event_stream) {
                return http_client_->StreamChunk(request, {.line_breaker = line_breaker})
                    | rpp::operators::map(details::strip_data_stream_prefix)
                    | rpp::operators::take_while([&,end_sentinels](const auto& chunk_string) {
                        return !details::is_end_sentinels(chunk_string, end_sentinels);
                    })
                    | rpp::operators::map([&](const auto& chunk_string) {
                        return converter_.Deserialize<ResponseEntity>(chunk_string);
                    });
            }

            return http_client_->StreamChunk(request, {.line_breaker = line_breaker})
                | rpp::operators::map([&](const auto& chunk_string) {
                    return converter_.Deserialize<ResponseEntity>(chunk_string);
                });
        }

    };

} // core


#endif //HTTPRESTCLIENT_H
