//
// Created by RobinQu on 2024/2/26.
//

#ifndef HTTPCHUNKRESULTITERATOR_HPP
#define HTTPCHUNKRESULTITERATOR_HPP

#include <boost/beast.hpp>
#include <utility>

#include "CoreGlobals.hpp"
#include "HttpChunkConnection.hpp"
#include "ResultIterator.hpp"

namespace INSTINCT_CORE_NS {
    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http; // from <boost/beast/http.hpp>
    namespace net = boost::asio; // from <boost/asio.hpp>
    using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>

    template <typename Chunk>
    class HttpChunkResultIterator: public ResultIterator<Chunk> {
    public:
        using ChunkConverter = std::function<Chunk(std::string)>;
        using TCPStremaProvider = std::function<beast::tcp_stream*()>;
        // using IdType = typename HttpChunkConnection<Chunk>::ChunkCache::size_type;

        HttpChunkResultIterator()=delete;

        HttpChunkResultIterator(
                ChunkConverter chunk_convert,
                TCPStremaProvider strema_provider
        ): chunk_convert_(std::move(chunk_convert)), strema_provider_(std::move(strema_provider)) {
            EnsureConnection();
        }

        ~HttpChunkResultIterator() {
            delete connection_;
        }

        [[nodiscard]] bool HasNext() const override {
            return connection_->hasNext();
        }

        Chunk& Next() override  {
            // while (connection_->HasNext()) {
            //     auto tmp = StringUtils::Trim(connection_->Next());
            //     if(!tmp.empty()) {
            //         return chunk_convert_(tmp);
            //     }
            // }
            data_.push_back(chunk_convert_(connection_->ReadNext()));
            return data_.back();
        }

    private:
        ChunkConverter chunk_convert_;
        TCPStremaProvider strema_provider_;
        HttpChunkConnection* connection_ = nullptr;
        // IdType current_ {0};
        std::vector<Chunk> data_;


        void EnsureConnection() {
            if(!connection_) {
                connection_ = new HttpChunkConnection(strema_provider_());
            }
        }
    };



}


#endif //HTTPCHUNKRESULTITERATOR_HPP
