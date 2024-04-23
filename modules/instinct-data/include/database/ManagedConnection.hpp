//
// Created by RobinQu on 2024/4/22.
//

#ifndef BASECONNECTION_HPP
#define BASECONNECTION_HPP

#include "IConnectionPool.hpp"
#include "tools/StringUtils.hpp"

namespace INSTINCT_DATA_NS {
    template<typename Impl>
    class ManagedConnection final: public IConnection<Impl>, public std::enable_shared_from_this<ManagedConnection<Impl>> {
        std::chrono::time_point<std::chrono::system_clock> last_active_time_point_;
        std::unique_ptr<Impl> impl_;
        std::string id_;
    public:
        explicit ManagedConnection(std::unique_ptr<Impl> impl_, std::string id = StringUtils::GenerateUUIDString())
            : last_active_time_point_(std::chrono::system_clock::now()), impl_(std::move(impl_)), id_(std::move(id)) {
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

        [[nodiscard]] const std::string & GetId() const override {
            return id_;
        }
    };
}


#endif //BASECONNECTION_HPP
