//
// Created by RobinQu on 2024/2/24.
//

#ifndef CHUNKSTREAM_HPP
#define CHUNKSTREAM_HPP

#include "CoreGlobals.hpp"
#include <boost/beast.hpp>
#include <utility>

#include "HttpRequest.hpp"
#include "HttpChunkIterator.hpp"
#include "SimpleHttpClient.hpp"


LC_CORE_NS {
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http; // from <boost/beast/http.hpp>
    namespace net = boost::asio; // from <boost/asio.hpp>
    using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

    using TCPStremaProvider = std::function<beast::tcp_stream*()>;


    template<typename Chunk>
    class HttpChunkStream {
    public:
        //using iterator = typename std::vector<Chunk>::iterator;
        using iterator = HttpChunkIterator<Chunk>;
        using const_iterator = HttpChunkIterator<const Chunk>;
        using ChunkConverter = std::function<Chunk(std::string)>;

        HttpChunkStream()=delete;
        HttpChunkStream(HttpChunkStream&&)=delete;
        HttpChunkStream(const HttpChunkStream&)=delete;

        HttpChunkStream(TCPStremaProvider strema_provider, ChunkConverter converter);
        ~HttpChunkStream();

        /**
         * \brief
         * \return will send actual request to target every time invoked
         */
        [[nodiscard]] auto begin();

        /**
         * \brief
         * \return return empty iterator
         */
        [[nodiscard]] auto end();


        [[nodiscard]] auto cbegin() const;

        [[nodiscard]] auto cend() const;



    private:

        ChunkConverter chunk_convert_;
        TCPStremaProvider strema_provider_;
        std::unordered_map<beast::tcp_stream*, HttpChunkConnection<Chunk>*> connections_;
    };

    template<typename Chunk>
    HttpChunkStream<Chunk>::~HttpChunkStream() {
        for(auto& [stream, connection]: connections_) {
            delete connection;
        }
    }

    template<typename Chunk>
    HttpChunkStream<Chunk>::HttpChunkStream(TCPStremaProvider strema_provider, ChunkConverter converter): chunk_convert_(std::move(converter)), strema_provider_(std::move(strema_provider)){

    }

    template<typename Chunk>
    auto HttpChunkStream<Chunk>::begin() {
        auto* stream = strema_provider_();
        return HttpChunkIterator<Chunk>(new HttpChunkConnection<Chunk>(stream, chunk_convert_));
    }

    template<typename Chunk>
    auto HttpChunkStream<Chunk>::end() {
        return HttpChunkIterator<Chunk>(nullptr);
    }

    template<typename Chunk>
    auto HttpChunkStream<Chunk>::cbegin() const {
        return begin();
    }

    template<typename Chunk>
    auto HttpChunkStream<Chunk>::cend() const {
        return end();
    }
}


#endif //CHUNKSTREAM_HPP
