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
        std::unique_ptr<Impl> impl_;
    public:
        ManagedConnection(const std::shared_ptr<IConnectionPool<Impl>> &connection_pool, std::unique_ptr<Impl> impl_)
            : connection_pool_(connection_pool), last_active_time_point_(std::chrono::system_clock::now()), impl_(std::move(impl_)) {
        }

        Impl& GetImpl() override {
            return *impl_;
        }

        Impl* operator->() const override {
            return impl_.get();
        }

        std::chrono::time_point<std::chrono::system_clock> GetLastActiveTime() override {
            return last_active_time_point_;
        }

        void UpdateActiveTime() override {
            last_active_time_point_ = std::chrono::system_clock::now();
        }


        ~ManagedConnection() override {
            if (auto ptr = connection_pool_.lock(); ptr) {
                ptr->Release(this->shared_from_this());
            }
        }
    };
}


#endif //BASECONNECTION_HPP
