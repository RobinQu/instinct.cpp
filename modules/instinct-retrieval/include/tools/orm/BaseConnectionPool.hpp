//
// Created by RobinQu on 2024/4/22.
//

#ifndef COMMOMCONNECTIONPOOL_HPP
#define COMMOMCONNECTIONPOOL_HPP

#include "IConnectionPool.hpp"

namespace INSTINCT_RETRIEVAL_NS {
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
        std::shared_ptr<IConnection<Impl>> Acquire() override {
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
                conn->IsAlive() &&
                conn->GetLastActiveTime() - std::chrono::system_clock::now() < options_.max_idle_duration) {
                conn->UpdateActiveTime();
                return conn;
            }

            return this->Create();
        }

        void Release(const std::shared_ptr<IConnection<Impl>> &connection) override {
            if (!connection || !connection->IsAlive() ) {
                connection = this->Create();
            }
            pool_.push_back(connection);
            condition_.notify_one();
        }

        bool Check(const typename IConnectionPool<Impl>::ConnectionPtr &connection) override {
            return true;
        }
    };



}



#endif //COMMOMCONNECTIONPOOL_HPP
