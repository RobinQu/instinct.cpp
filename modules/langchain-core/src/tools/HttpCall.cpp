//
// Created by RobinQu on 2024/2/1.
//

#include "HttpCall.h"

#include "StringUtils.h"
#include <fmt/format.h>

#include "HttpUtils.h"


#include <boost/url.hpp>

namespace langchain::core {

    HttpCall::HttpCall(const std::string& request_line) {
        // parse request line
        std::vector<std::string> parts = langchian::core::StringUtils::Resplit(request_line);
        if(parts.size() != 2) {
            throw LangchainException(fmt::format("invalid request line: {}", request_line));
        }
        method_ = HttpUtils::ParseMethod(parts[0]);
        if(auto uri_result = boost::urls::parse_uri_reference(parts[1]); uri_result) {
            boost::url url = *uri_result;
            endpoint_.host = url.host();
            endpoint_.port = url.port_number();
            path_ = url.path();
            query_string_ = url.query();
            target_ = path_ + "?" + query_string_;
        } else {
            const boost::system::error_code e = uri_result.error();
            throw LangchainException(fmt::format("invalid url: {}", e.to_string()));
        }
    }

    HttpCall::HttpCall(const std::string& host, int port, const std::string& target, const HttpMethod& method, const HttpHeaders& headers): endpoint_(host, port), method_(method), headers_(headers), target_(target), query_string_(), path_() {
    }

    Endpoint HttpCall::GetEndpoint() const {
        return endpoint_;
    }

    std::string HttpCall::GetPath() const {
        return path_;
    }

    std::string HttpCall::GetQueryString() const {
        return query_string_;
    }

    std::string HttpCall::GetTarget() const {
        return target_;
    }

    HttpMethod HttpCall::GetMethod() const {
        return method_;
    }

    HttpHeaders HttpCall::GetHeaders() const {
        return headers_;
    }
} // core
// langchain