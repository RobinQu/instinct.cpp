//
// Created by RobinQu on 2024/4/22.
//

#ifndef BASECONNECTION_HPP
#define BASECONNECTION_HPP

#include "IConnectionPool.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    template<typename Impl>
    class ManagedConnection: public IConnection<Impl>, public std::enable_shared_from_this<ManagedConnection<Impl>> {
        std::weak_ptr<IConnectionPool<Impl>> connection_pool_;
        std::chrono::time_point<std::chrono::system_clock> last_active_time_point_;
        Impl impl_;
    public:
        ManagedConnection(const std::shared_ptr<IConnectionPool<Impl>> &connection_pool, Impl impl_)
            : connection_pool_(connection_pool), last_active_time_point_(std::chrono::system_clock::now()), impl_(std::move(impl_)) {
        }

        Impl & GetImpl() const override {
            return impl_;
        }

        Impl* operator->() const override {
            return *impl_;
        }


        std::chrono::time_point<std::chrono::system_clock> GetLastActiveTime() override {
            return last_active_time_point_;
        }

        void UpdateActiveTime() override {
            last_active_time_point_ = std::chrono::system_clock::now();
        }


        ~ManagedConnection() override {
            if (connection_pool_.lock()) {
                connection_pool_->Release(this->shared_from_this());
            }
        }
    };
}


#endif //BASECONNECTION_HPP
