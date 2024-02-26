//
// Created by RobinQu on 2024/2/24.
//

#ifndef HTTPCHUNKCONNECTION_HPP
#define HTTPCHUNKCONNECTION_HPP
#include <boost/beast.hpp>

#include "CoreGlobals.hpp"
#include <vector>

#include "CoreTypes.hpp"
#include <mutex>
#include <exception>

#include "StringUtils.hpp"

LC_CORE_NS {


    namespace beast = boost::beast;     // from <boost/beast.hpp>
    namespace http = beast::http;       // from <boost/beast/http.hpp>
    namespace net = boost::asio;        // from <boost/asio.hpp>
    using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>


    class HttpChunkConnection {
    public:
        // using ChunkConverter = std::function<ChunkType(std::string)>;
        // using ChunkCache = ChunkCacheType;
        // using ChunkCacheIdx = typename ChunkCache::size_type;

        explicit HttpChunkConnection(beast::tcp_stream* stream);
        ~HttpChunkConnection();
        // HttpChunkConnection()=delete;
        HttpChunkConnection(HttpChunkConnection&&)=delete;
        HttpChunkConnection(const HttpChunkConnection&)=delete;

        std::string ReadNext();
        [[nodiscard]] bool hasNext() const;
        // std::string& GetChunk(ChunkCacheIdx idx);

        bool operator==(const HttpChunkConnection&) const;
        bool operator!=(const HttpChunkConnection&) const;

    private:
        // owning pointer to stream
        beast::tcp_stream* stream_;
        // ChunkConverter chunk_converter_;
        // std::vector<ChunkType> cache_;

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

        std::mutex read_mutex_;
    };


    inline HttpChunkConnection::~HttpChunkConnection() {
        delete stream_;
    }


    inline bool HttpChunkConnection::operator==(const HttpChunkConnection&rhs) const {
        return this->stream_ == rhs.stream_;
    }


    inline bool HttpChunkConnection::operator!=(const HttpChunkConnection&rhs) const {
        return !(*this==rhs);
    }

    inline HttpChunkConnection::HttpChunkConnection(beast::tcp_stream* const stream): stream_(stream) {
        beast::error_code ec_;
        http::read_header(*stream, buffer_, parser_, ec_);
        if(ec_) {
            throw LangchainException("failed to read header: " + ec_.message());
        }
        if(!parser_.chunked()) {
            throw LangchainException("response is not chunk-encoding");
        }

        header_callback_ =
        [&](std::uint64_t size,         // Size of the chunk, or zero for the last chunk
            boost::string_view extensions,     // The raw chunk-extensions string. Already validated.
            boost::system::error_code& ev)             // We can set this to indicate an error
        {
            // Parse the chunk extensions so we can access them easily
            ce_.parse(extensions, ev);
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
            chunk_.reserve(static_cast<std::size_t>(size));
            chunk_.clear();
        };

        parser_.on_chunk_header(header_callback_);


        body_callback_ = [&](std::uint64_t remain,   // The number of bytes left in this chunk
            boost::string_view body,       // A buffer holding chunk body data
            boost::system::error_code& ec)         // We can set this to indicate an error
        {
            // If this is the last piece of the chunk body,
            // set the error so that the call to `read` returns
            // and we can process the chunk.
            if(remain == body.size())
                ec = http::error::end_of_chunk;

            // Append this piece to our container
            chunk_.append(body.data(), body.size());

            // The return value informs the parser of how much of the body we
            // consumed. We will indicate that we consumed everything passed in.
            return body.size();
        };
        parser_.on_chunk_body(body_callback_);
    }


    inline std::string HttpChunkConnection::ReadNext()  {
        // make sure only one thread is reading underlying stream at the same time
        std::lock_guard<std::mutex> guard(read_mutex_);

        boost::system::error_code ec;
        while(!parser_.is_done()) {
            http::read(*stream_, buffer_, parser_, ec);
            if(! ec)
                continue;
            else if(ec != http::error::end_of_chunk)
                throw LangchainException("chunk parsing exception: " + ec.message());
            else
                ec.assign(0, ec.category());

            break;
        }
        // try {
        //     cache_.emplace_back(chunk_converter_(
        //         langchian::core::StringUtils::Trim(chunk_)
        //     ));
        // } catch (const std::runtime_error &e) {
        //     throw LangchainException(e, "ChunkConverter error");
        // } catch (const std::exception& e) {
        //     throw LangchainException(e.what());
        // }

        if(parser_.is_done()) {
            delete stream_;
            // mark as nullptr so that iterator knows it's finished
            stream_ = nullptr;
        }

        return chunk_;
    }

    inline bool HttpChunkConnection::hasNext() const {
        return stream_ && !parser_.is_done();
    }

    using HttpChunkConnectionPtr= std::shared_ptr<HttpChunkConnection>;
}

#endif //HTTPCHUNKCONNECTION_HPP
