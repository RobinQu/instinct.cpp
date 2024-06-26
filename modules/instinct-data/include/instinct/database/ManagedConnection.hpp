//
// Created by RobinQu on 2024/4/22.
//

#ifndef BASECONNECTION_HPP
#define BASECONNECTION_HPP

#include "IConnectionPool.hpp"
#include "tools/StringUtils.hpp"

namespace INSTINCT_DATA_NS {
    template<typename ConnectionImpl, typename  QueryResultImpl>
    class ManagedConnection: public IConnection<ConnectionImpl, QueryResultImpl>, public std::enable_shared_from_this<ManagedConnection<ConnectionImpl, QueryResultImpl>> {
        std::chrono::time_point<std::chrono::system_clock> last_active_time_point_;
        std::unique_ptr<ConnectionImpl> impl_;
        std::string id_;
        std::shared_ptr<inja::Environment> env_;
    public:
        explicit ManagedConnection(std::unique_ptr<ConnectionImpl> impl_, std::string id, std::shared_ptr<inja::Environment> template_env)
            : last_active_time_point_(std::chrono::system_clock::now()), impl_(std::move(impl_)), id_(std::move(id)), env_(std::move(template_env)) {
        }

        ConnectionImpl& GetImpl() override {
            return *impl_;
        }

        ConnectionImpl* operator->() const override {
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

        QueryResultImpl Query(const SQLTemplate &select_sql, const SQLContext &context) override {
            const auto sql_line = env_->render(select_sql, context);
            LOG_DEBUG("Query: {}", sql_line);
            return Execute(sql_line);
        }

    private:
        virtual QueryResultImpl Execute(const std::string& sql_line) = 0;
    };
}


#endif //BASECONNECTION_HPP
