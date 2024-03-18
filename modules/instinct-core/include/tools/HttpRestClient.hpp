//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H


#include <google/protobuf/util/json_util.h>

#include "Assertions.hpp"
#include "CoreGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

#include "HttpClientException.hpp"
#include "SimpleHttpClient.hpp"


namespace INSTINCT_CORE_NS {


    namespace details {
        struct ProtobufHttpEntityConverter {
            template<typename T>
            requires is_pb_message<T>
            T Deserialize(const std::string& buf) {
                T result;
                auto status = google::protobuf::util::JsonStringToMessage(buf, &result);
                assert_true(status.ok(), "failed to parse protobuf message from response body");
                return result;
            }

            template<typename T>
            requires is_pb_message<T>
            std::string Serialize(const T& obj) {
                std::string param_string;
                auto status = google::protobuf::util::MessageToJsonString(obj, &param_string);
                assert_true(status.ok(), "failed to dump parameters from protobuf message");
                return param_string;
            }
        };

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




class HttpRestClient final: public SimpleHttpClient {
    details::ProtobufHttpEntityConverter converter_;

public:
    HttpRestClient() = delete;

    explicit HttpRestClient(const Endpoint& endpoint)
        : SimpleHttpClient(endpoint), converter_() {
    }

    template<typename ResponseEntity>
    ResponseEntity GetObject(const std::string& uri) {
        const HttpRequest request = {kGET, uri, {
                {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
        }, ""};
        const HttpResponse response = Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        return converter_.Deserialize<ResponseEntity>(response.body);
    }

    template<typename RequestEntity, typename ResponseEntity>
    ResponseEntity PostObject(const std::string& uri, const RequestEntity& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {kPOST, uri, {
                {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
        }, param_string};
        const auto [headers, body, status_code] = Execute(request);
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
            .method = kPOST,
            .target =uri,
            .headers = {
                {HTTP_HEADER_CONTENT_TYPE_NAME, HTTP_CONTENT_TYPES.at(kJSON) }
            },
            .body = param_string
        };

        if (is_sse_event_stream) {
            return StreamChunk(request)
                | rpp::operators::map(details::strip_data_stream_prefix)
                | rpp::operators::take_while([&,end_sentinels](const auto& chunk_string) {
                    return !details::is_end_sentinels(chunk_string, end_sentinels);
                })
                | rpp::operators::map([&](const auto& chunk_string) {
                    std::cout << "chunk: " <<  chunk_string << std::endl;
                    return converter_.Deserialize<ResponseEntity>(chunk_string);
                });
        }

        return StreamChunk(request)
            | rpp::operators::map([&](const auto& chunk_string) {
                return converter_.Deserialize<ResponseEntity>(chunk_string);
            });
    }

};

} // core


#endif //HTTPRESTCLIENT_H
