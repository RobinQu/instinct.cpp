//
// Created by RobinQu on 2024/1/14.
//

#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <boost/beast.hpp>
#include "CoreTypes.hpp"
#include "HttpChunkStream.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"


using namespace boost::beast;


LC_CORE_NS {
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http; // from <boost/beast/http.hpp>
    namespace net = boost::asio; // from <boost/asio.hpp>
    using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

    struct HttpClientOptions {
        unsigned int timeout = 6;
    };

    static std::string string_idenity_fn(std::string v) {
        return v;
    }

    class SimpleHttpClient {
        net::io_context ioc_;
        Endpoint endpoint_;
        HttpClientOptions options_;
        tcp::resolver::results_type resolve_results_;

    public:
        SimpleHttpClient() = delete;

        explicit SimpleHttpClient(Endpoint endpoint);

        SimpleHttpClient(Endpoint endpoint, HttpClientOptions options);

        HttpResponse Execute(
            const HttpRequest& call
        );

        template<typename Chunk, typename ChunkConverter=std::function<Chunk(std::string)>>
        HttpChunkStream<Chunk> Stream(
            const HttpRequest& call,
            ChunkConverter&& converter
        ) ;

        [[nodiscard]] auto Stream(const HttpRequest& call)  {
            return this->template Stream<std::string>(call, string_idenity_fn);
        }

    };

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
        if (verb == http::verb::unknown) {
            throw LangchainException("unknown http verb");
        }
        return verb;
    }

    inline SimpleHttpClient::SimpleHttpClient(Endpoint endpoint): SimpleHttpClient(std::move(endpoint), {}) {
    }

    inline SimpleHttpClient::SimpleHttpClient(Endpoint endpoint, HttpClientOptions options): ioc_(),
        endpoint_(std::move(endpoint)), options_(options), resolve_results_() {
        tcp::resolver resolver(ioc_);
        resolve_results_ = resolver.resolve(
            endpoint.host,
            std::to_string(endpoint.port)
        );
    }


    inline HttpResponse SimpleHttpClient::Execute(const HttpRequest& call) {
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
        for (auto& h: res.base()) {
            response_headers.emplace(h.name_string(), h.value());
        }
        // HttpResponse response {res.result_int(), const_cast<HttpHeaders&>(response_headers), body_string};
        HttpResponse response{
            response_headers,
            buffers_to_string(res.body().data()),
            res.result_int()
        };
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec && ec != beast::errc::not_connected) {
            throw LangchainException(fmt::format("TCP socket shutdown error: {}", ec.message()));
        }
        return response;
    }


    template<typename Chunk, typename ChunkConverter>
    HttpChunkStream<Chunk> SimpleHttpClient::Stream(
            const HttpRequest& call,
            ChunkConverter&& converter
        )  {
        return HttpChunkStream<Chunk>([&,call]() {
            auto stream = new beast::tcp_stream(ioc_);
            stream->connect(resolve_results_);
            http::verb verb = parse_verb(call);
            http::request<http::string_body> req{verb, call.target, 11};
            req.set(http::field::host, endpoint_.host);
            req.body() = call.body;
            req.prepare_payload();
            http::write(*stream, req);
            return stream;
        }, std::forward<ChunkConverter>(converter));
    }

} // core
// langchain

#endif //SIMPLEHTTPCLIENT_H
