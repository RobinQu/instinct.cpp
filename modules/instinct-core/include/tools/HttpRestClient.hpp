//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H




#include <utility>

#include "Assertions.hpp"
#include "CoreGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

#include "tools/http/HttpClientException.hpp"
#include "tools/http/IHttpClient.hpp"
#include "tools/http/CURLHttpClient.hpp"
#include "tools/ProtobufUtils.hpp"



namespace INSTINCT_CORE_NS {
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

    }

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


class HttpRestClient final {
    ProtobufHttpEntityConverter converter_;
    Endpoint endpoint_;
    HttpClientPtr http_client_;

public:
    HttpRestClient() = delete;

    explicit HttpRestClient(Endpoint endpoint)
        : endpoint_(std::move(endpoint)), converter_() {
        http_client_ = CreateCURLHttpClient();
    }

    template<typename ResponseEntity>
    ResponseEntity GetObject(const std::string& uri) {
        const HttpRequest request = {
                endpoint_,
                kGET,
                uri,
                {
                {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
                }
        };
        const HttpResponse response = http_client_->Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        return converter_.Deserialize<ResponseEntity>(response.body);
    }

    template<typename RequestEntity, typename ResponseEntity>
    ResponseEntity PostObject(const std::string& uri, const RequestEntity& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {
                endpoint_,
                kPOST,
                uri,
                {
                    {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON)
                    }
        }, param_string};
        const auto [headers, body, status_code] = http_client_->Execute(request);
        if(status_code >= 400) {
            throw HttpClientException(status_code, body);
        }
        return converter_.Deserialize<ResponseEntity>(body);
    }

    template<typename RequestEntity, typename ResponseEntity>
    AsyncIterator<ResponseEntity> StreamChunkObject(
        const std::string& uri,
        const RequestEntity& param,
        bool is_sse_event_stream = false,
        const std::vector<std::string>& end_sentinels = {}
    ) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {
            .endpoint = endpoint_,
            .method = kPOST,
            .target = uri,
            .headers = {
                {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
            },
            .body = param_string
        };

        if (is_sse_event_stream) {
            return http_client_->StreamChunk(request)
                | rpp::operators::map(details::strip_data_stream_prefix)
                | rpp::operators::take_while([&,end_sentinels](const auto& chunk_string) {
                    return !details::is_end_sentinels(chunk_string, end_sentinels);
                })
                | rpp::operators::map([&](const auto& chunk_string) {
//                    std::cout << "chunk: " <<  chunk_string << std::endl;
                    return converter_.Deserialize<ResponseEntity>(chunk_string);
                });
        }

        return http_client_->StreamChunk(request)
            | rpp::operators::map([&](const auto& chunk_string) {
                return converter_.Deserialize<ResponseEntity>(chunk_string);
            });
    }

};

} // core


#endif //HTTPRESTCLIENT_H
