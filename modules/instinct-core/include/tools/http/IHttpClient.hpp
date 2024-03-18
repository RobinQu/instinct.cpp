//
// Created by RobinQu on 2024/3/18.
//

#ifndef INSTINCT_IHTTPCLIENT_HPP
#define INSTINCT_IHTTPCLIENT_HPP

#include "CoreGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

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

    struct HttpRequest {
        HttpMethod method = HttpMethod::kGET;
        std::string target;
        HttpHeaders headers;
        std::string body;
    };

    struct HttpResponse {
        HttpHeaders headers;
        std::string body;
        unsigned int status_code;
    };



    class IHttpClient {
    public:
        virtual ~IHttpClient() = default;
        IHttpClient(const IHttpClient&)=delete;
        IHttpClient(IHttpClient&&)=delete;

        virtual HttpResponse Execute(
                const HttpRequest& call
        ) = 0;

        virtual AsyncIterator<std::string> StreamChunk(
                const HttpRequest& call
        ) = 0;


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




#endif //INSTINCT_IHTTPCLIENT_HPP
