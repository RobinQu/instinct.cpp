//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H

#include "CoreTypes.hpp"
#include "HttpClientException.hpp"
#include "SimpleHttpClient.hpp"


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
        const HttpResponse response = Execute(request);
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        return nlohmann::json::parse(response.body);
    }
    template<typename Param, typename Result>
    Result PostObject(const std::string& uri, const Param& param) {
        const nlohmann::json json_object = param;
        const HttpRequest request = {POST, uri, {}, json_object.dump()};
        // std::cout << "Request body: " <<  request.body << std::endl;
        const HttpResponse response = Execute(request);
        // std::cout << "Response body: " << response->body << std::endl;
        if(response.status_code >= 400) {
            throw HttpClientException(response.status_code, response.body);
        }
        auto response_body_json = nlohmann::json::parse(response.body);
        return response_body_json.get<Result>();
    }

    template<typename Param, typename Result>
    ResultIterator<Result>* StreamChunk(const std::string& uri, const Param& param) {
        const nlohmann::json json_object = param;
        const HttpRequest request = {POST, uri, {}, json_object.dump()};
        return create_transform([](auto&& v) {
            const auto trimed = langchian::core::StringUtils::Trim(v);
            if(trimed.empty()) {
                return Result {};
            }
            auto response_body_json = nlohmann::json::parse(trimed);
            return response_body_json.template get<Result>();
        }, Stream(request));
    }
};

} // core
// langchain

#endif //HTTPRESTCLIENT_H
