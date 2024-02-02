//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H
#include "SimpleHttpClient.h"


namespace langchain::core {

class HttpRestClient final: SimpleHttpClient {
public:
    template<typename Result>
    Result GetObject(const std::string& uri) {
        const HttpRequest request = HttpRequest::FromReuqestLine("GET " + uri);
        const HttpResponse response = DoExecute(request);
        return nlohmann::json::parse(response.body);
    }
    template<typename Param, typename Result>
    Result PostObject(const std::string& uri, const Param& param) {
        HttpRequest request = HttpRequest::FromReuqestLine("POST " + uri);
        const nlohmann::json json_object = param;
        request.body = json_object.dump();
        const HttpResponse response = DoExecute(request);
        auto response_body_json = nlohmann::json::parse(response.body);
        return response_body_json.get<Result>();
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
