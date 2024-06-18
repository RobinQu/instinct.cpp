//
// Created by RobinQu on 2024/3/18.
//

#ifndef INSTINCT_IHTTPCLIENT_HPP
#define INSTINCT_IHTTPCLIENT_HPP

#include "CoreGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

namespace INSTINCT_CORE_NS {
    using HttpHeaders = std::unordered_map<std::string, std::string>;
    using HttpQueryParamters = std::unordered_map<std::string, std::string>;


    enum MIMEContentType {
        kKnownContentType,
        kJSON,
        kEventStream,
        kPlainText
    };

    static const std::unordered_map<MIMEContentType, std::string> HTTP_CONTENT_TYPES = {
            {kJSON, "application/json"},
            {kEventStream, "text/event-stream"},
            {kPlainText, "text/plain"}
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
        kUnspecifiedProtocol,
        kHTTP,
        kHTTPS
    };

    static const std::map<std::string, HttpProtocol> protocol_map{
            {"http", kHTTP},
            {"https", kHTTPS}
    };

    struct Endpoint {
        HttpProtocol protocol = kUnspecifiedProtocol;
        std::string host;
        int port = 0;
    };

    struct HttpRequest {
        Endpoint endpoint;
        HttpMethod method = kGET;
        std::string target;
        HttpHeaders headers;
        std::string body;
        HttpQueryParamters parameters;
    };

    struct HttpResponse {
        HttpHeaders headers;
        std::string body;
        unsigned int status_code = 0;
    };

    struct HttpStreamResponse {
        HttpHeaders headers;
        unsigned int status_code = 0;
    };

    /**
     * Callback function that will be used inside write function. Return false to stop this http session.
     */
    using HttpResponseCallback = std::function<bool(std::string)>;

    struct StreamChunkOptions {
        std::string line_breaker;
    };

    class IHttpClient {
    public:
        virtual ~IHttpClient() = default;
        IHttpClient(const IHttpClient&)=delete;
        IHttpClient(IHttpClient&&)=delete;
        IHttpClient() = default;

        virtual HttpResponse Execute(
                const HttpRequest& call
        ) = 0;

        virtual HttpStreamResponse ExecuteWithCallback(const HttpRequest& call, const HttpResponseCallback& callback) = 0;

        /**
         * Run batch of http requests. Concurrency behaviour (max parallel, ...) is controlled by given thread pool.
         * @param calls
         * @param pool thread pool for concurrent execution
         * @return 
         */
        virtual Futures<HttpResponse> ExecuteBatch(
            const std::vector<HttpRequest>& calls,
            ThreadPool& pool
        ) = 0;

        virtual AsyncIterator<std::string> StreamChunk(
            const HttpRequest& call,
            const StreamChunkOptions& options
        ) = 0;
    };
    using HttpClientPtr = std::shared_ptr<IHttpClient>;
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
            case INSTINCT_CORE_NS::kUnspecifiedProtocol: name = ""; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};




#endif //INSTINCT_IHTTPCLIENT_HPP
