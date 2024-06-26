//
// Created by RobinQu on 2024/4/22.
//

#ifndef COMMOMCONNECTIONPOOL_HPP
#define COMMOMCONNECTIONPOOL_HPP

#include "IConnectionPool.hpp"

namespace INSTINCT_DATA_NS {
    using namespace std::chrono_literals;

    struct ConnectionPoolOptions {
        int initial_connection_count = 5;
        std::chrono::minutes max_idle_duration = 60min;
        std::chrono::seconds max_wait_duration_for_acquire = 3s;
    };



    template<typename ConnectionImpl, typename  QueryResultImpl>
    class BaseConnectionPool: public IConnectionPool<ConnectionImpl, QueryResultImpl>, public std::enable_shared_from_this<BaseConnectionPool<ConnectionImpl, QueryResultImpl>>{
        std::deque<std::shared_ptr<IConnection<ConnectionImpl, QueryResultImpl>>> pool_;
        std::mutex mutex_;
        std::condition_variable condition_;
        ConnectionPoolOptions options_;
    public:
        using ConnectionPtr = typename IConnectionPool<ConnectionImpl, QueryResultImpl>::ConnectionPtr;
        using ConnectionPoolPtr = std::shared_ptr<BaseConnectionPool<ConnectionImpl, QueryResultImpl>>;

        /**
         * Helper class to guard a connection to released in end of scope
         * @tparam Impl
         */
        struct GuardConnection {
            ConnectionPoolPtr pool_;
            std::weak_ptr<IConnection<ConnectionImpl, QueryResultImpl>> connection_;
            ~GuardConnection() {
                if (auto ptr = connection_.lock(); ptr) {
                    pool_->Release(ptr);
                }
            }
        };

        explicit BaseConnectionPool(const ConnectionPoolOptions &options)
            : options_(options) {
        }

        void Initialize() override {
            auto count = options_.initial_connection_count;
            LOG_INFO("Init pool with {} connections", count);
            while (count-->0) {
                pool_.push_back(this->Create());
            }
        }

        ConnectionPtr TryAcquire() override {
            std::unique_lock lock(mutex_);
            while (pool_.empty()) {
                if (condition_.wait_for(lock, options_.max_wait_duration_for_acquire) ==
                    std::cv_status::timeout) {
                    // timeout
                    return nullptr;
                    }
            }
            auto conn = pool_.front();
            pool_.pop_front();

            if (conn &&
                this->Check(conn) &&
                conn->GetLastActiveTime() - std::chrono::system_clock::now() < options_.max_idle_duration) {
                conn->UpdateActiveTime();
                return conn;
            }

            return this->Create();
        }

        ConnectionPtr Acquire() override {
            if (const auto conn = TryAcquire()) {
                LOG_DEBUG("Acquired: {}", conn->GetId());
                return conn;
            }
            throw InstinctException("Cannot acquire connection from connection pool");
        }

        void Release(const ConnectionPtr &connection) override {
            if (!connection || !this->Check(connection) ) {
                auto c = this->Create();
                LOG_DEBUG("Discarded previous broken connection. Newly created: {}", c->GetId());
                pool_.push_back(c);
            } else {
                LOG_DEBUG("Released: {}", connection->GetId());
                pool_.push_back(connection);
            }
            condition_.notify_one();
        }

    };

    template<typename ConnectionImpl, typename  QueryResultImpl>
    using ConnectionPoolPtr = std::shared_ptr<BaseConnectionPool<ConnectionImpl, QueryResultImpl>>;



}



#endif //COMMOMCONNECTIONPOOL_HPP
