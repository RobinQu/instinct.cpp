//
// Created by RobinQu on 2024/2/1.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <string>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/args.h>
#include <uriparser/Uri.h>
#include <curl/curl.h>
#include <fmt/ranges.h>


#include "IHttpClient.hpp"
#include "tools/StringUtils.hpp"
#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    using namespace fmt::literals;


    struct HttpUtils {

        static void AssertHttpRequest(const HttpRequest& request) {
            assert_true(!StringUtils::IsBlankString(request.endpoint.host), "host cannot be blank.");
            assert_true(request.endpoint.port != 0, "port canot be zero");
            assert_true(request.endpoint.protocol != kUnspecifiedProtocol, "protocol should be either HTTP or HTTPS");
        }

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
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(fmt::arg("protocol", call.endpoint.protocol));
            store.push_back(fmt::arg("host", call.endpoint.host));
            store.push_back(fmt::arg("port", call.endpoint.port));
            store.push_back(fmt::arg("target", call.target));
            if (!call.parameters.empty()) {
                std::vector<std::string> parameter_pairs;
                for (const auto& [k,v]: call.parameters) {
                    parameter_pairs.push_back(fmt::format("{}={}", k, curl_easy_escape(nullptr, v.c_str(), static_cast<int>(v.size()))));
                }
                store.push_back(fmt::arg("query_string", StringUtils::JoinWith(parameter_pairs, "&")));
                return fmt::vformat("{protocol}://{host}:{port}{target}?{query_string}", store);
            }

            return fmt::vformat("{protocol}://{host}:{port}{target}", store);
        }

        static HttpRequest CreateRequest(const std::string& request_line) {
            HttpRequest call;
            // parse request line
            std::vector<std::string> parts = StringUtils::ReSplit(request_line);
            if(parts.size() != 2) {
                throw InstinctException(fmt::format("invalid request line: {}", request_line));
            }
            call.method = ParseMethod(parts[0]);
            UriUriA  uri;
            const char* error_pos;
            if(uriParseSingleUriA(&uri, parts[1].data(), &error_pos) == URI_SUCCESS) {
                auto scheme = std::string {uri.scheme.first, uri.scheme.afterLast};

                auto port_text = std::string {uri.portText.first, uri.portText.afterLast};
                if (scheme == "http") {
                    call.endpoint.protocol = kHTTP;
                    if(port_text.empty()) {
                        call.endpoint.port = 80;
                    }
                } else if(scheme == "https") {
                    call.endpoint.protocol = kHTTPS;
                    if(port_text.empty()) {
                        call.endpoint.port = 443;
                    }
                } else { // other protocols are not currently supported
                    call.endpoint.protocol = kUnspecifiedProtocol;
                }
                if(!port_text.empty()) { // override port number if specifically defined
                    call.endpoint.port = std::stoi(port_text);
                }
                call.endpoint.host = std::string {uri.hostText.first, uri.hostText.afterLast};

                auto path_head = uri.pathHead;
                while(path_head) {
                    call.target += "/";
                    call.target += {path_head->text.first, path_head->text.afterLast};
                    path_head = path_head->next;
                }

                // query paratmeters
                call.parameters = ParseQueryParameters({uri.query.first, uri.query.afterLast});
            }
            uriFreeUriMembersA(&uri);
            return call;
        }

        static HttpQueryParamters ParseQueryParameters(const std::string& query_string) {
            HttpQueryParamters paramters;
            if (!query_string.empty()) {
                for(const auto& part: StringUtils::ReSplit(query_string, std::regex{R"(&)"})) {
                    auto kv_pair = StringUtils::ReSplit(part, std::regex{R"(=)"});
                    assert_true(kv_pair.size() == 2, "illegal query parameter pair: " + part);
                    int k_out_len = 0, v_out_len = 0;
                    auto unescaped_k = curl_easy_unescape(nullptr, kv_pair[0].data(), kv_pair[0].size(), &k_out_len);
                    auto unescaped_v = curl_easy_unescape(nullptr, kv_pair[1].data(), kv_pair[1].size(), &v_out_len);
                    paramters[std::string{unescaped_k, static_cast<size_t>(k_out_len)}] = std::string {unescaped_v, static_cast<size_t>(v_out_len)};
                }
            }
            return paramters;
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
