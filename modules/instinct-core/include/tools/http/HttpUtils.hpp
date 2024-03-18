//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include "tools/StringUtils.hpp"

#include "CoreGlobals.hpp"
#include <string>
#include "fmt/ranges.h"
#include "IHttpClient.hpp"
#include <fmt/core.h>


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

        static std::string CreateUrlString(const HttpRequest& call) {

        }

        static HttpRequest FromRequestLine(const std::string& request_line) {
            HttpRequest call;
            // parse request line
            std::vector<std::string> parts = StringUtils::ReSplit(request_line);
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
