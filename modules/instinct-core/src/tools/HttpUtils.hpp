//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include "StringUtils.hpp"

#include "CoreGlobals.hpp"
#include <string>
#include "fmt/ranges.h"



namespace INSTINCT_CORE_NS {
    using namespace fmt::literals;

    typedef std::unordered_map<std::string, std::string> HttpHeaders;

    enum HttpMethod {
        Unkown,
        GET,
        POST,
        PUT,
        DELETE,
        HEAD
    };


    struct Endpoint {
        std::string host;
        int port;
    };

    struct HttpUtils {
        static HttpMethod ParseMethod(const std::string& str) {
            auto m = StringUtils::ToUpper(str);
            if(m == "PUT") return HttpMethod::PUT;
            if(m == "GET") return HttpMethod::GET;
            if(m == "POST") return HttpMethod::POST;
            if(m == "DELETE") return DELETE;
            if(m == "HEAD") return HEAD;
            return Unkown;
        }
    };
}


template <> struct fmt::formatter<INSTINCT_CORE_NS::HttpMethod>: fmt::formatter<fmt::string_view> {
    template <typename FormatContext>
    auto format(INSTINCT_CORE_NS::HttpMethod c, FormatContext& ctx) {
        fmt::string_view name = "unknown";
        switch (c) {
            case INSTINCT_CORE_NS::POST:   name = "POST"; break;
            case INSTINCT_CORE_NS::GET: name = "GET"; break;
            case INSTINCT_CORE_NS::PUT:  name = "PUT"; break;
            case INSTINCT_CORE_NS::DELETE:  name = "DELETE"; break;
            case INSTINCT_CORE_NS::HEAD:  name = "HEAD"; break;
            case INSTINCT_CORE_NS::Unkown: name = "UNKNOWN"; break;
        }
        return formatter<fmt::string_view>::format(name, ctx);
    }
};


#endif //HTTPUTILS_H
