//
// Created by RobinQu on 2024/2/13.
//

#ifndef HTTPCLIENTEXCEPTION_H
#define HTTPCLIENTEXCEPTION_H
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"

LC_CORE_NS {

class HttpClientException: public LangchainException {
public:
    const unsigned int status_code;
    const std::string raw_response;


    HttpClientException(const unsigned int status_code, std::string raw_response): LangchainException("HttpClientException with status_code="+std::to_string(status_code)), status_code(status_code), raw_response(std::move(raw_response)) {

    }

    HttpClientException(const std::string& basic_string, const unsigned int status_code, std::string raw_response)
        : LangchainException(basic_string),
          status_code(status_code),
          raw_response(std::move(raw_response)) {
    }
};

} // LC_CORE_NS

#endif //HTTPCLIENTEXCEPTION_H
