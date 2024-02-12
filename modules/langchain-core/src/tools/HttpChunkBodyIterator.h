//
// Created by RobinQu on 2024/2/9.
//

#ifndef HTTPCHUNKBODYITERATOR_H
#define HTTPCHUNKBODYITERATOR_H
#include <boost/beast.hpp>

namespace langchain::core {

    namespace beast = boost::beast;     // from <boost/beast.hpp>
    namespace http = beast::http;       // from <boost/beast/http.hpp>
    namespace net = boost::asio;        // from <boost/asio.hpp>
    using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

    class HttpChunkBodyIterator {
        // owning pointer to stream
        beast::tcp_stream* stream_;

        http::parser<false, http::empty_body> parser_;
        std::string chunk_;
        beast::flat_buffer buffer_;
        http::chunk_extensions ce_;

        std::function<void(std::uint64_t,
            boost::string_view,
            boost::system::error_code&)> header_callback_;

        std::function<std::size_t(std::uint64_t,
            boost::string_view,
            boost::system::error_code&)> body_callback_;

    public:
        HttpChunkBodyIterator() = delete;
        HttpChunkBodyIterator(const HttpChunkBodyIterator &) = delete;
        ~HttpChunkBodyIterator();
        explicit HttpChunkBodyIterator(
            beast::tcp_stream* stream
            // http::parser<false, http::empty_body>* parser_
        );
        [[nodiscard]] bool HasNext() const;
        [[nodiscard]] std::string Next();
    };

    using HttpChunkBodyIteratorPtr = std::shared_ptr<HttpChunkBodyIterator>;

}





#endif //HTTPCHUNKBODYITERATOR_H
