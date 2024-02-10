//
// Created by RobinQu on 2024/2/6.
//
#include <gtest/gtest.h>
#include <boost/beast.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


template<bool isRequest>
class custom_parser
    : public http::basic_parser<isRequest>
{
private:
    // The friend declaration is needed,
    // otherwise the callbacks must be made public.
    friend class http::basic_parser<isRequest>;


protected:
    void on_request_impl(http::verb method, beast::string_view method_str, beast::string_view target, int version,
        beast::error_code& ec) override;

    void on_response_impl(int code, beast::string_view reason, int version, beast::error_code& ec) override;

    void on_field_impl(http::field name, beast::string_view name_string, beast::string_view value,
        beast::error_code& ec) override;

    std::size_t on_body_impl(beast::string_view body, beast::error_code& ec) override;

    void on_chunk_header_impl(std::uint64_t size, beast::string_view extensions, beast::error_code& ec) override;

    std::size_t on_chunk_body_impl(std::uint64_t remain, beast::string_view body, beast::error_code& ec) override;

    void on_header_impl(beast::error_code& ec) override;

    void on_body_init_impl(const boost::optional<std::uint64_t>& content_length, beast::error_code& ec) override;

    void on_finish_impl(beast::error_code& ec) override;

public:
    custom_parser() = default;
};

template<bool isRequest>
void custom_parser<isRequest>::on_request_impl(http::verb method, beast::string_view method_str,
    beast::string_view target, int version, beast::error_code& ec) {
    std::cout << "on_request" << std::endl;
}

template<bool isRequest>
void custom_parser<isRequest>::
on_response_impl(int code, beast::string_view reason, int version, beast::error_code& ec) {
    std::cout << "on_response" << std::endl;
}

template<bool isRequest>
void custom_parser<isRequest>::on_field_impl(http::field name, beast::string_view name_string, beast::string_view value,
    beast::error_code& ec) {
    std::cout << "on_field, name="  << name_string << std::endl;
}

template<bool isRequest>
std::size_t custom_parser<isRequest>::on_body_impl(beast::string_view body, beast::error_code& ec) {
    std::cout << "body=" << body << std::endl;
    return body.size();
}

template<bool isRequest>
void custom_parser<isRequest>::on_chunk_header_impl(std::uint64_t size, beast::string_view extensions,
    beast::error_code& ec) {
}

template<bool isRequest>
std::size_t custom_parser<isRequest>::on_chunk_body_impl(std::uint64_t remain, beast::string_view body,
    beast::error_code& ec) {
    return 0;
}

template<bool isRequest>
void custom_parser<isRequest>::on_header_impl(beast::error_code& ec) {
    std::cout << "on_header_impl" << std::endl;
}

template<bool isRequest>
void custom_parser<isRequest>::on_body_init_impl(const boost::optional<std::uint64_t>& content_length,
    beast::error_code& ec) {
    std::cout << "on_body_init_impl, content_length=" << content_length.get_value_or(0) << std::endl;
}

template<bool isRequest>
void custom_parser<isRequest>::on_finish_impl(beast::error_code& ec) {
    std::cout << "on_finish_impl" << std::endl;
}

TEST(Beast, CustomParserTest1) {
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    auto const results = resolver.resolve("localhost", "11434");
    stream.connect(results);

    http::request<http::string_body> req{http::verb::post, "/api/generate", 11};
    req.set(http::field::host, "localhost:11434");
    req.body() = R"({
        "model": "llama2",
        "prompt": "Why is the sky blue?"
    })";
    req.prepare_payload();
    http::write(stream, req);

    custom_parser<false> p;
    beast::flat_buffer buffer;
    while(!p.is_done()) {
        http::read(stream, buffer, p);
    }
}