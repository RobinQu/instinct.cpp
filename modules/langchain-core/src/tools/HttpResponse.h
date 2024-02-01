//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include "Types.h"


namespace langchain::core {

class HttpResponse {

    HttpHeaders headers_;
    std::string body_;
    unsigned int status_code_;

public:
    HttpResponse(
        unsigned int status_code,
        const HttpHeaders& headers,
        const std::string& body
    );

    HttpHeaders& GetHeaders();
    [[nodiscard]] HttpHeaders GetHeaders() const;

    std::string& GetBody();
    [[nodiscard]] std::string GetBody() const;

    [[nodiscard]] unsigned int GetStatusCode() const;


};

} // core
// langchain

#endif //HTTPRESPONSE_H
