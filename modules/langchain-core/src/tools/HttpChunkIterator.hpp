//
// Created by RobinQu on 2024/2/24.
//

#ifndef HTTPCHUNKITERATOR_HPP
#define HTTPCHUNKITERATOR_HPP

#include "CoreGlobals.hpp"
#include "HttpChunkConnection.hpp"

LC_CORE_NS {

    namespace beast = boost::beast; // from <boost/beast.hpp>
    namespace http = beast::http; // from <boost/beast/http.hpp>
    namespace net = boost::asio; // from <boost/asio.hpp>
    using tcp = net::ip::tcp; // from <boost/asio/ip/tcp.hpp>
    using StringCache = std::vector<std::string>;


    template<typename Chunk>
    class HttpChunkIterator {
        using IdType = typename HttpChunkConnection<Chunk>::ChunkCache::size_type;
        using ConnectionType = HttpChunkConnection<Chunk>;

        ConnectionType* chunk_connection_;
        IdType current_ {0};

    public:
        using value_type = Chunk;
        using difference_type = std::ptrdiff_t;
        using ponter_type = Chunk *;
        using reference_type = Chunk &;
        using iterator_category = std::input_iterator_tag;

        explicit  HttpChunkIterator(ConnectionType* connection): chunk_connection_(connection) {}

        reference_type operator*() const {
            chunk_connection_->ReadNextOf(current_);
            return chunk_connection_->GetChunk(current_);
        }

        HttpChunkIterator& operator++() {
            ++current_;
            return *this;
        }

        HttpChunkIterator operator++(int) {
            HttpChunkIterator tmp = *this;
            ++current_;
            return tmp;
        }

        HttpChunkIterator& operator--() {
            --current_;
            return *this;

        }

        HttpChunkIterator operator--(int) {
            HttpChunkIterator tmp = *this;
            --current_;
            return tmp;
        }

        bool operator==(const HttpChunkIterator& rhs) {
            if(this->chunk_connection_) {
                if(rhs.chunk_connection_) { // both are valid pointer
                    return this->chunk_connection_->GetChunk(current_) == rhs.chunk_connection_->GetChunk(current_);
                }
                // return false;
                // if connection is empty, then it should be treated as nullptr
                return !this->chunk_connection_->hasNext();
            }
            if(!rhs.chunk_connection_) { // both are nullptr
                return true;
            }
            // rhs is valid pointer, but this->chunk_connection_ is nullptr. if its connection is empty, then treat it as nullptr
            return !rhs.chunk_connection_->hasNext();
        }

        bool operator!=(const HttpChunkIterator& rhs) {
            return !(*this == rhs);
        }
    };




}
#endif //HTTPCHUNKITERATOR_HPP
