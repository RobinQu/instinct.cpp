//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include "StringUtils.h"
#include "CoreTypes.h"


namespace langchain::core {
    struct HttpUtils {
        static HttpMethod ParseMethod(const std::string& str) {
            auto m = langchian::core::StringUtils::ToUpper(str);
            if(m == "PUT") return HttpMethod::PUT;
            if(m == "GET") return HttpMethod::GET;
            if(m == "POST") return HttpMethod::POST;
            if(m == "DELETE") return DELETE;
            if(m == "HEAD") return HEAD;
            return Unkown;
        }
    };
}

#endif //HTTPUTILS_H
