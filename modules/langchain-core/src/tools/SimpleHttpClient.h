//
// Created by RobinQu on 2024/1/14.
//

#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <boost/beast.hpp>

#include "CoreTypes.h"
#include "HttpChunkBodyIterator.h"
#include "HttpRequest.h"
#include "HttpResponse.h"


using namespace boost::beast;


namespace langchain::core {

    namespace beast = boost::beast;     // from <boost/beast.hpp>
    namespace http = beast::http;       // from <boost/beast/http.hpp>
    namespace net = boost::asio;        // from <boost/asio.hpp>
    using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


    struct HttpClientOptions {
        int timeout = 6;
    };


    class SimpleHttpClient {
        net::io_context ioc_;
        Endpoint endpoint_;
        HttpClientOptions options_;
         tcp::resolver::results_type resolve_results_;
    public:
        SimpleHttpClient() = delete;
        explicit SimpleHttpClient(Endpoint endpoint);
        SimpleHttpClient(Endpoint endpoint, HttpClientOptions options);

        HttpResponsePtr Execute(
            const HttpRequest& call
        );

        HttpChunkBodyIteratorPtr Stream(
            const HttpRequest& call
        );

    };

} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
