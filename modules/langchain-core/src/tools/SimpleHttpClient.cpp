//
// Created by RobinQu on 2024/1/14.
//

#include "SimpleHttpClient.h"
#include <fmt/format.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace langchain {

namespace core {


    namespace beast = boost::beast;     // from <boost/beast.hpp>
    namespace http = beast::http;       // from <boost/beast/http.hpp>
    namespace net = boost::asio;        // from <boost/asio.hpp>
    using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


    HttpResponse SimpleHttpClient::DoExecute(const HttpRequest& call) {
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);
        // auto endpoint = call.GetEndpoint();
        auto const results = resolver.resolve(
            call.host,
            std::to_string(call.port)
            );
        stream.connect(results);
        http::verb verb;
        switch (call.method) {
            case GET:
                verb = http::verb::get;
                break;
            case POST:
                verb = http::verb::post;
                break;
            case PUT:
                verb = http::verb::put;
                break;
            case HEAD:
                verb = http::verb::head;
                break;
            default:
                verb = http::verb::unknown;
        }
        if(verb == http::verb::unknown) {
            throw LangchainException("unknown http verb");
        }
        http::request<http::string_body> req{verb, call.target, 11};
        req.set(http::field::host, call.host);
        req.body() = call.body;
        req.prepare_payload();
        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        // std::string body = res.body();


        HttpHeaders response_headers;
        for(auto& h: res.base()) {
            response_headers.emplace(h.name_string(), h.value());
        }
        // HttpResponse response {res.result_int(), const_cast<HttpHeaders&>(response_headers), body_string};
        HttpResponse response;
        response.body = buffers_to_string(res.body().data());
        response.headers = response_headers;
        response.status_code = res.result_int();

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected) {
            throw LangchainException(fmt::format("TCP socket shutdown error: {}", ec.message()));
        }
        return response;
    }
} // core
} // langchain