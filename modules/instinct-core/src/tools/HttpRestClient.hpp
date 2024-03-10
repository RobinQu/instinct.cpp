//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include "Assertions.hpp"
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "HttpClientException.hpp"
#include "SimpleHttpClient.hpp"


namespace INSTINCT_CORE_NS {

class HttpRestClient final: public SimpleHttpClient {
public:
    HttpRestClient() = delete;

    HttpRestClient(const std::string& host, const int port)
        : SimpleHttpClient({.host = host, .port = port}) {
    }

    explicit HttpRestClient(Endpoint endpoint)
        : SimpleHttpClient(std::move(endpoint)) {
    }

    template<typename Result>
    requires is_pb_message<Result>
    Result GetObject(const std::string& uri) {
        const HttpRequest request = {GET, uri, {}, ""};
        const HttpResponse response = Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        Result result;
        auto status = google::protobuf::util::JsonStringToMessage(response.body, &result);
        assert_true(status.ok(), "failed to parse protobuf message from response body");
        return result;
    }

    template<typename Param, typename Result>
    requires is_pb_message<Result> && is_pb_message<Param>
    Result PostObject(const std::string& uri, const Param& param) {
        std::string param_string;
        auto status = google::protobuf::util::MessageToJsonString(param, &param_string);
        assert_true(status.ok(), "failed to dump parameters from protobuf message");

        const HttpRequest request = {POST, uri, {}, param_string};
        // std::cout << "Request body: " <<  request.body << std::endl;
        const HttpResponse response = Execute(request);
        // std::cout << "Response body: " << response->body << std::endl;
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }

        Result result;
        status = google::protobuf::util::JsonStringToMessage(response.body, &result);
        assert_true(status.ok(), "failed to parse protobuf message from response body");
        return result;
    }

    template<typename Param, typename Result>
    requires is_pb_message<Result> && is_pb_message<Param>
    ResultIteratorPtr<Result> StreamChunk(const std::string& uri, const Param& param) {
        std::string param_string;
        auto status = google::protobuf::util::MessageToJsonString(param, &param_string);
        assert_true(status.ok(), "failed to dump parameters from protobuf message");

        const HttpRequest request = {POST, uri, {}, param_string};
        return create_result_itr_with_transform([](const std::string& v)-> Result {
            const auto trimed = StringUtils::Trim(v);
            Result result;
            if(trimed.empty()) {
                return result;
            }
            auto to_message_status = google::protobuf::util::JsonStringToMessage(trimed, &result);
            assert_true(to_message_status.ok(), "failed to parse protobuf message from response body");
            return result;
        }, Stream(request));
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
