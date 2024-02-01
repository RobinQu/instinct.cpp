//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include "Types.h"


namespace langchain::core {

struct HttpResponse {

    HttpHeaders headers;
    std::string body;
    unsigned int status_code;
};

} // core
// langchain

#endif //HTTPRESPONSE_H
