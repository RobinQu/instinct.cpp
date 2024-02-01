//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPCALL_H
#define HTTPCALL_H
#include "Types.h"

namespace langchain {
namespace core {

class HttpCall {
    Endpoint endpoint_;
    std::string path_;
    std::string query_string_;
    std::string target_;
    HttpMethod method_;
    HttpHeaders headers_;
public:
    explicit HttpCall(const std::string& request_line);
    HttpCall(
        const std::string& host,
        int port,
        const std::string& target,
        const HttpMethod& method,
        const HttpHeaders& headers
        );

    [[nodiscard]] Endpoint GetEndpoint() const;
    [[nodiscard]] std::string GetPath() const;
    [[nodiscard]] std::string GetQueryString() const;
    [[nodiscard]] std::string GetTarget() const;
    [[nodiscard]] HttpMethod GetMethod() const;
    [[nodiscard]] HttpHeaders GetHeaders() const;

};

} // core
} // langchain

#endif //HTTPCALL_H
