//
// Created by RobinQu on 2024/1/14.
//

#include "SimpleHttpClient.h"
#include <fmt/format.h>

#include "HttpResponse.h"

namespace langchain {

namespace core {


    namespace beast = boost::beast;     // from <boost/beast.hpp>
    namespace http = beast::http;       // from <boost/beast/http.hpp>
    namespace net = boost::asio;        // from <boost/asio.hpp>
    using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


    void SimpleHttpClient::DoExecute(const HttpCall& call, const std::function<std::string()>& request_body_function,
        const std::function<void(const HttpResponse&)>& response_body_function) {
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);
        auto endpoint = call.GetEndpoint();
        auto const results = resolver.resolve(
            endpoint.host,
            std::to_string(endpoint.port)
            );
        stream.connect(results);
        http::verb verb;
        switch (call.GetMethod()) {
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
        http::request<http::string_body> req{verb, call.GetTarget(), 11};
        req.set(http::field::host, endpoint.host);
        req.body() = request_body_function();
        req.prepare_payload();
        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        // std::string body = res.body();


        std::string body_string = boost::beast::buffers_to_string(res.body());
        HttpHeaders response_headers;
        for(auto& h: res.base()) {
            response_headers.insert(h.name_string(), h.value());
        }
        HttpResponse response(res.result_int(), response_headers, body_string);
        response_body_function(response);

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected) {
            throw LangchainException(fmt::format("TCP socket shutdown error: {}", ec.message()));
        }
    }
} // core
} // langchain