//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include "StringUtils.hpp"

#include "CoreGlobals.hpp"
#include <string>
#include "fmt/ranges.h"
#include <fmt/core.h>

namespace INSTINCT_CORE_NS {
    typedef std::unordered_map<std::string, std::string> HttpHeaders;

    enum MIMEContentType {
        kKnownContentType,
        kJSON
    };

    static const std::unordered_map<MIMEContentType, std::string> HTTP_CONTENT_TYPES = {
        {kJSON, "application/json"}
    };
    static const std::string HTTP_HEADER_CONTENT_TYPE_NAME = "Content-Type";

    enum HttpMethod {
        kUnspecifiedHttpMethod,
        kGET,
        kPOST,
        kPUT,
        kDELETE,
        kHEAD
    };

    enum HttpProtocol {
        kSpecifiedProtocol,
        kHTTP,
        kHTTPS
    };

    struct Endpoint {
        HttpProtocol protocol = kHTTP;
        std::string host;
        int port;
    };
}


template <> struct fmt::formatter<INSTINCT_CORE_NS::HttpMethod>: formatter<string_view> {
    template <typename FormatContext>
    auto format(INSTINCT_CORE_NS::HttpMethod c, FormatContext& ctx) {
        string_view name = "UNKNOWN";
        switch (c) {
            case INSTINCT_CORE_NS::kPOST:   name = "POST"; break;
            case INSTINCT_CORE_NS::kGET: name = "GET"; break;
            case INSTINCT_CORE_NS::kPUT:  name = "PUT"; break;
            case INSTINCT_CORE_NS::kDELETE:  name = "DELETE"; break;
            case INSTINCT_CORE_NS::kHEAD:  name = "HEAD"; break;
            case INSTINCT_CORE_NS::kUnspecifiedHttpMethod: name = "UNKNOWN"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};


template <> struct fmt::formatter<INSTINCT_CORE_NS::HttpProtocol>: formatter<string_view> {
    template <typename FormatContext>
    auto format(INSTINCT_CORE_NS::HttpProtocol c, FormatContext& ctx) {
        string_view name = "";
        switch (c) {
            case INSTINCT_CORE_NS::kHTTP:   name = "http"; break;
            case INSTINCT_CORE_NS::kHTTPS: name = "https"; break;
            case INSTINCT_CORE_NS::kSpecifiedProtocol: name = ""; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};

namespace INSTINCT_CORE_NS {
    using namespace fmt::literals;


    struct HttpUtils {
        static HttpMethod ParseMethod(const std::string& str) {
            auto m = StringUtils::ToUpper(str);
            if(m == "PUT") return kPUT;
            if(m == "GET") return kGET;
            if(m == "POST") return kPOST;
            if(m == "DELETE") return kDELETE;
            if(m == "HEAD") return kHEAD;
            return kUnspecifiedHttpMethod;
        }

        static std::string CreateConnectionString(const Endpoint& endpoint) {
            return fmt::format("{}://{}:{}", endpoint.protocol, endpoint.host, endpoint.port);
        }

        static std::string GetHeaderValue(const std::string& name, const std::string& default_value, const HttpHeaders& headers) {
            if (headers.contains(name)) {
                return headers.at(name);
            }
            if (auto lowered_name = StringUtils::ToLower(name); headers.contains(lowered_name)) {
                return headers.at(lowered_name);
            }
            if (auto upper_name = StringUtils::ToUpper(name); headers.contains(upper_name)) {
                return headers.at(upper_name);
            }
            return default_value;
        }
    };


}




#endif //HTTPUTILS_H
