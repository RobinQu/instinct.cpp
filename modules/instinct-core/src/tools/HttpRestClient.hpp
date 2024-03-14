//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "Assertions.hpp"
#include "CoreGlobals.hpp"

#include "HttpClientException.hpp"
#include "SimpleHttpClient.hpp"


namespace INSTINCT_CORE_NS {

struct ProtobufHttpEntityConverter {
    template<typename T>
    requires is_pb_message<Result>
    T Deserialize(const std::string& buf) {
        T result;
        auto status = google::protobuf::util::JsonStringToMessage(buf, &result);
        assert_true(status.ok(), "failed to parse protobuf message from response body");
        return result;
    }

    template<typename T>
    requires is_pb_message<Result>
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

    template<typename Result>
    Result GetObject(const std::string& uri) {
        const HttpRequest request = {kGET, uri, {}, ""};
        const HttpResponse response = Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        return converter_.Deserialize<Result>(response.body);
    }

    template<typename Param, typename Result>
    Result PostObject(const std::string& uri, const Param& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {kPOST, uri, {}, param_string};
        const auto& [headers, body, status_code] = Execute(request);
        if(status_code >= 400) {
            throw HttpClientException(status_code, body);
        }
        return converter_.Deserialize<Result>(body);
    }

    template<typename Param, typename Result>
    auto StreamChunkObject(const std::string& uri, const Param& param) {
        std::string param_string = converter_.Serialize(param);
        const HttpRequest request = {kPOST, uri, {}, param_string};
        return StreamChunk(request) | rpp::operators::map([&](const auto& chunk_string) {
            return converter_.Deserialize<Result>(chunk_string);
        });
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
