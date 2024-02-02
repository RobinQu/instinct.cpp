//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPCALL_H
#define HTTPCALL_H
#include "Types.h"


namespace langchain::core {

struct HttpRequest {
    static HttpRequest FromReuqestLine(const std::string& request_line);
    std::string host;
    int port;
    std::string path;
    std::string query_string;
    std::string target;
    HttpMethod method;
    HttpHeaders headers;
    std::string body;
};

} // core
// langchain

#endif //HTTPCALL_H
