//
// Created by RobinQu on 2024/1/14.
//

#include "SimpleHttpClient.h"
#include <fmt/format.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace langchain {

namespace core {

    static http::verb parse_verb(const HttpRequest& call) {
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
        return verb;
    }

    SimpleHttpClient::SimpleHttpClient(Endpoint endpoint): SimpleHttpClient(std::move(endpoint), {}) {
    }

    SimpleHttpClient::SimpleHttpClient(Endpoint endpoint, HttpClientOptions options): ioc_(), endpoint_(std::move(endpoint)), options_(options), resolve_results_() {
        tcp::resolver resolver(ioc_);
        resolve_results_ = resolver.resolve(
            endpoint.host,
            std::to_string(endpoint.port)
            );
    }


    HttpResponsePtr SimpleHttpClient::DoExecute(const HttpRequest& call) {
        beast::tcp_stream stream(ioc_);
        stream.connect(resolve_results_);
        http::verb verb = parse_verb(call);
        http::request<http::string_body> req{verb, call.target, 11};
        req.set(http::field::host, endpoint_.host);
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
        HttpResponsePtr response = std::make_shared<HttpResponse>();
        response->body = buffers_to_string(res.body().data());
        response->headers = response_headers;
        response->status_code = res.result_int();

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected) {
            throw LangchainException(fmt::format("TCP socket shutdown error: {}", ec.message()));
        }
        return response;
    }

    HttpChunkBodyIteratorPtr SimpleHttpClient::Stream(const HttpRequest& call) {
        auto* stream = new beast::tcp_stream(ioc_);
        stream->connect(resolve_results_);
        http::verb verb = parse_verb(call);
        http::request<http::string_body> req{verb, call.target, 11};
        req.set(http::field::host, endpoint_.host);
        req.body() = call.body;
        req.prepare_payload();
        http::write(*stream, req);
        // read header to ensure that response is chunked-encoding
        return std::make_shared<HttpChunkBodyIterator>(stream);
    }


} // core
} // langchain