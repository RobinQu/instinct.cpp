//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include "CoreTypes.hpp"


namespace langchain::core {

struct HttpResponse {
    HttpHeaders headers;
    std::string body;
    unsigned int status_code;
};
using HttpResponsePtr = std::shared_ptr<HttpResponse>;

} // core
// langchain

#endif //HTTPRESPONSE_H
