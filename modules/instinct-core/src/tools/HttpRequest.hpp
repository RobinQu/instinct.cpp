//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPCALL_H
#define HTTPCALL_H
#include "CoreTypes.hpp"
#include "HttpUtils.hpp"


namespace INSTINCT_CORE_NS {

struct HttpRequest {
    static HttpRequest FromReuqestLine(const std::string& request_line);
    // std::string host;
    // int port;
    // std::string path;
    // std::string query_string;
    HttpMethod method = HttpMethod::GET;
    std::string target;
    HttpHeaders headers;
    std::string body;
};

    inline HttpRequest HttpRequest::FromReuqestLine(const std::string& request_line) {
        HttpRequest call;
        // parse request line
        std::vector<std::string> parts = StringUtils::Resplit(request_line);
        if(parts.size() != 2) {
            throw InstinctException(fmt::format("invalid request line: {}", request_line));
        }
        call.method = HttpUtils::ParseMethod(parts[0]);
        call.target = parts[1];
        // if(auto uri_result = boost::urls::parse_uri_reference(parts[1]); uri_result) {
        //     boost::url url = *uri_result;
        //     call.host = url.host();
        //     call.port = url.port_number();
        //     call.path = url.path();
        //     call.query_string = url.query();
        //     call.target = call.path + "?" + call.query_string;
        // } else {
        //     const boost::system::error_code e = uri_result.error();
        //     throw InstinctException(fmt::format("invalid url: {}", e.to_string()));
        // }
        return call;
    }

} // core
// langchain

#endif //HTTPCALL_H
