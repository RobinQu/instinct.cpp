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


class HttpRestClient final: public SimpleHttpClient {
    ProtobufHttpEntityConverter converter_;
public:
    HttpRestClient() = delete;

    explicit HttpRestClient(const Endpoint& endpoint)
        : SimpleHttpClient(endpoint), converter_() {
    }

    template<typename ResponseEntity>
    ResponseEntity GetObject(const std::string& uri) {
        const HttpRequest request = {kGET, uri, {}, ""};
        const HttpResponse response = Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        return converter_.Deserialize<ResponseEntity>(response.body);
    }

    template<typename RequestEntity, typename ResponseEntity>
    ResponseEntity PostObject(const std::string& uri, const RequestEntity& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {kPOST, uri, {}, param_string};
        const auto [headers, body, status_code] = Execute(request);
        if(status_code >= 400) {
            throw HttpClientException(status_code, body);
        }
        return converter_.Deserialize<ResponseEntity>(body);
    }

    template<typename RequestEntity, typename ResponseEntity>
    AsyncIterator<ResponseEntity> StreamChunkObject(const std::string& uri, const RequestEntity& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {kPOST, uri, {}, param_string};
        return StreamChunk(request) | rpp::operators::map([&](const auto& chunk_string) {
            return converter_.Deserialize<ResponseEntity>(chunk_string);
        });
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
