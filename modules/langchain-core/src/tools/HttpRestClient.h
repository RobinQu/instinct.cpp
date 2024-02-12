//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H
#include "SimpleHttpClient.h"


namespace langchain::core {

class HttpRestClient final: public SimpleHttpClient {
public:
    HttpRestClient() = delete;

    explicit HttpRestClient(Endpoint endpoint)
        : SimpleHttpClient(std::move(endpoint)) {
    }

    template<typename Result>
    Result GetObject(const std::string& uri) {
        const HttpRequest request = {GET, uri, {}, ""};
        const HttpResponsePtr response = Execute(request);
        return nlohmann::json::parse(response->body);
    }
    template<typename Param, typename Result>
    Result PostObject(const std::string& uri, const Param& param) {
        const nlohmann::json json_object = param;
        const HttpRequest request = {POST, uri, {}, json_object.dump()};
        const HttpResponsePtr response = Execute(request);
        auto response_body_json = nlohmann::json::parse(response->body);
        return response_body_json.get<Result>();
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
