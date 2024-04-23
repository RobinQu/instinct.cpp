//
// Created by RobinQu on 2024/4/22.
//

#ifndef COMMOMCONNECTIONPOOL_HPP
#define COMMOMCONNECTIONPOOL_HPP

#include "IConnectionPool.hpp"

namespace INSTINCT_DATA_NS {
    using namespace std::chrono_literals;

    struct ConnectionPoolOptions {
        int intial_connection_count = 5;
        std::chrono::minutes max_idle_duration = 60min;
        std::chrono::seconds max_wait_duration_for_acquire = 3s;
    };



    template<typename Impl>
    class BaseConnectionPool: public IConnectionPool<Impl>, public std::enable_shared_from_this<BaseConnectionPool<Impl>>{
        std::deque<std::shared_ptr<IConnection<Impl>>> pool_;
        std::mutex mutex_;
        std::condition_variable condition_;
        ConnectionPoolOptions options_;
    public:

        /**
         * Helper class to guard a connection to released in end of scope
         * @tparam Impl
         */
        struct GuardConnection {
            std::shared_ptr<IConnectionPool<Impl>> pool_;
            std::weak_ptr<IConnection<Impl>> connection_;


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
            auto count = options_.intial_connection_count;
            LOG_INFO("Init pool with {} connections", count);
            while (count-->0) {
                pool_.push_back(this->Create());
            }
        }

        std::shared_ptr<IConnection<Impl>> TryAcquire() override {
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

        typename IConnectionPool<Impl>::ConnectionPtr Acquire() override {
            if (const auto conn = TryAcquire()) {
                LOG_DEBUG("Acquired one: {}", conn->GetId());
                return conn;
            }
            throw InstinctException("Cannot acquire connection from connection pool");
        }

        void Release(const std::shared_ptr<IConnection<Impl>> &connection) override {
            if (!connection || !this->Check(connection) ) {
                auto c = this->Create();
                LOG_DEBUG("Discard one. Newly created: {}", c->GetId());
                pool_.push_back(c);
            } else {
                LOG_DEBUG("Release one: {}", connection->GetId());
                pool_.push_back(connection);
            }
            condition_.notify_one();
        }

    };

    template<typename Impl>
    using ConnectionPoolPtr = std::shared_ptr<BaseConnectionPool<Impl>>;



}



#endif //COMMOMCONNECTIONPOOL_HPP
