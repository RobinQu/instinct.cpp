//
// Created by RobinQu on 2024/2/26.
//

#ifndef HTTPCHUNKRESULTITERATOR_HPP
#define HTTPCHUNKRESULTITERATOR_HPP

#include <boost/beast.hpp>

#include "CoreGlobals.hpp"
#include "HttpChunkConnection.hpp"
#include "ResultIterator.hpp"

LC_CORE_NS {
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http; // from <boost/beast/http.hpp>
    namespace net = boost::asio; // from <boost/asio.hpp>
    using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

    template <typename Chunk>
    class HttpChunkResultIterator: public ResultIterator<Chunk> {
    public:
        using ChunkConverter = std::function<Chunk(std::string)>;
        using TCPStremaProvider = std::function<beast::tcp_stream*()>;
        using IdType = typename HttpChunkConnection<Chunk>::ChunkCache::size_type;

        HttpChunkResultIterator()=delete;

        HttpChunkResultIterator(
                ChunkConverter chunk_convert_,
                TCPStremaProvider strema_provider_
        ): connection_(chunk_convert_, strema_provider_) {

        }

        [[nodiscard]] bool HasNext() const override {
            return connection_.hasNext();
        }

        Chunk& Next() const override  {
            ++current_;
            // FIXME: const hack
            const_cast<HttpChunkConnection<Chunk>&>(connection_).ReadNextOf(current_);
            return connection_.GetChunk();
        }

    private:
        // ChunkConverter chunk_convert_;
        // TCPStremaProvider strema_provider_;
        HttpChunkConnection<Chunk> connection_;
        IdType current_ {0};

    };



}


#endif //HTTPCHUNKRESULTITERATOR_HPP
