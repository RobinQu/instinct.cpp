//
// Created by RobinQu on 2024/2/5.
//
#include <gtest/gtest.h>
#include <boost/beast.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


template<
    bool isRequest,
    class SyncReadStream,
    class DynamicBuffer>
void
read_and_print_body(
    std::ostream& os,
    SyncReadStream& stream,
    DynamicBuffer& buffer,
    boost::system::error_code& ec)
{
    http::parser<isRequest, http::buffer_body> p;
    read_header(stream, buffer, p, ec);

    if(ec)
        return;
    while(! p.is_done())
    {
        char buf[512];
        p.get().body().data = buf;
        p.get().body().size = sizeof(buf);
        int s1 = sizeof(buf);
        read(stream, buffer, p, ec);
        if(ec == http::error::need_buffer)
            ec.assign(0, ec.category());
        if(ec)
            return;
        int s2 = sizeof(buf);
        int s3 = p.get().body().size;
        std::cout << "s1=" << s1 << ",s2=" << s2 << ",s3=" << s3 << std::endl;
        os.write(buf, sizeof(buf) - p.get().body().size);
    }
}

TEST(Beast, ParserTest1) {
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
        }
    )";
    req.prepare_payload();
    http::write(stream, req);


    beast::flat_buffer buffer;
    boost::system::error_code ec;
    read_and_print_body<false, beast::tcp_stream, beast::flat_buffer>(std::cout, stream, buffer, ec);
}



TEST(Beast, ChunkedEncoding) {
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
        }
    )";
    req.prepare_payload();
    http::write(stream, req);


    beast::flat_buffer buffer;
    boost::system::error_code ec;
    http::parser<false, http::empty_body> p;
    http::read_header(stream, buffer, p, ec);
    if(ec) return;
    http::chunk_extensions ce;
    std::string chunk;
    auto header_cb =
    [&](std::uint64_t size,         // Size of the chunk, or zero for the last chunk
        boost::string_view extensions,     // The raw chunk-extensions string. Already validated.
        boost::system::error_code& ev)             // We can set this to indicate an error
    {
        // Parse the chunk extensions so we can access them easily
        ce.parse(extensions, ev);
        if(ev)
            return;

        // See if the chunk is too big
        if(size > (std::numeric_limits<std::size_t>::max)())
        {
            ev = http::error::body_limit;
            return;
        }

        // Make sure we have enough storage, and
        // reset the container for the upcoming chunk
        chunk.reserve(static_cast<std::size_t>(size));
        chunk.clear();
    };

    p.on_chunk_header(header_cb);

    auto body_cb =
    [&](std::uint64_t remain,   // The number of bytes left in this chunk
        boost::string_view body,       // A buffer holding chunk body data
        boost::system::error_code& ec)         // We can set this to indicate an error
    {
        // If this is the last piece of the chunk body,
        // set the error so that the call to `read` returns
        // and we can process the chunk.
        if(remain == body.size())
            ec = http::error::end_of_chunk;

        // Append this piece to our container
        chunk.append(body.data(), body.size());

        // The return value informs the parser of how much of the body we
        // consumed. We will indicate that we consumed everything passed in.
        return body.size();
    };
    p.on_chunk_body(body_cb);

    while(! p.is_done())
    {
        // Read as much as we can. When we reach the end of the chunk, the chunk
        // body callback will make the read return with the end_of_chunk error.
        read(stream, buffer, p, ec);
        if(! ec)
            continue;
        else if(ec != http::error::end_of_chunk)
            return;
        else
            ec.assign(0, ec.category());

        // We got a whole chunk, print the extensions:
        for(auto const& extension : ce)
        {
            std::cout << "Extension: " << extension.first;
            if(! extension.second.empty())
                std::cout << " = " << extension.second << std::endl;
            else
                std::cout << std::endl;
        }

        // Now print the chunk body
        std::cout << "Chunk Body: " << chunk << std::endl;
    }

    // Get a reference to the parsed message, this is for convenience
    auto const& msg = p.get();
    // Check each field promised in the "Trailer" header and output it

    for(auto const& name : http::token_list{msg[http::field::trailer]})
    {
        // Find the trailer field
        auto it = msg.find(name);
        if(it == msg.end())
        {
            // Oops! They promised the field but failed to deliver it
            std::cout << "Missing Trailer: " << name << std::endl;
            continue;
        }
        std::cout << it->name() << ": " << it->value() << std::endl;
    }
}