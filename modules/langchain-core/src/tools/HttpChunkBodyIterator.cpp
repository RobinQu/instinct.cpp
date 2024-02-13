//
// Created by RobinQu on 2024/2/9.
//

#include "HttpChunkBodyIterator.h"

#include "CoreTypes.h"
#include <iostream>

namespace langchain::core {


    HttpChunkBodyIterator::HttpChunkBodyIterator(beast::tcp_stream* stream): stream_(stream), parser_(), chunk_(), buffer_(), ce_() {
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

    bool HttpChunkBodyIterator::HasNext() const {
        return !parser_.is_done();
    }

    std::string HttpChunkBodyIterator::Next() {
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
        // while chunk is parsed
        return chunk_;
    }

    HttpChunkBodyIterator::~HttpChunkBodyIterator() {
        // std::cout << "teardown of HttpChunkBodyIterator" << std::endl;
        delete stream_;
    }
}
