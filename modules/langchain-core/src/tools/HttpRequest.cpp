//
// Created by RobinQu on 2024/2/1.
//


#include "HttpRequest.h"

#include "StringUtils.h"
#include <fmt/format.h>

#include "HttpUtils.h"


#include <boost/url.hpp>

namespace langchain::core {

    HttpRequest HttpRequest::FromReuqestLine(const std::string& request_line) {
        HttpRequest call;
        // parse request line
        std::vector<std::string> parts = langchian::core::StringUtils::Resplit(request_line);
        if(parts.size() != 2) {
            throw LangchainException(fmt::format("invalid request line: {}", request_line));
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
        //     throw LangchainException(fmt::format("invalid url: {}", e.to_string()));
        // }
        return call;
    }
} // core
// langchain