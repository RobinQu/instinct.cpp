//
// Created by RobinQu on 2024/6/14.
//

#ifndef DBMIGRATION_HPP
#define DBMIGRATION_HPP
#include <cmrc/cmrc.hpp>

#include <instinct/AssistantGlobals.hpp>
#include <instinct/database/DBUtils.hpp>
#include <instinct/ioc/ManagedApplicationContext.hpp>

CMRC_DECLARE(instinct::assistant);


namespace INSTINCT_ASSISTANT_NS {
    using namespace  INSTINCT_DATA_NS;

    template<typename ConnectionImpl, typename  QueryResultImpl>
    class DBMigration final: public ILifeCycle {
        volatile bool running_ = false;
        std::filesystem::path db_file_path_;
        ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool_;

    public:
        DBMigration(std::filesystem::path db_file_path,
            ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool)
            : db_file_path_(std::move(db_file_path)),
              connection_pool_(std::move(connection_pool)) {
        }

        void Start() override {
            running_ = true;
            // db migration
            const auto fs = cmrc::instinct::assistant::get_filesystem();
            const auto sql_file = fs.open("db_migration/001/up.sql");
            const auto sql_line = std::string {sql_file.begin(), sql_file.end()};
            LOG_DEBUG("Initialize database at {} with sql:\n {}", db_file_path_, sql_line);
            DBUtils::ExecuteSQL(sql_line, connection_pool_);
        }

        void Stop() override {
            running_ = false;
        }

        u_int32_t GetPriority() override {
            return HIGH_PRIORITY;
        }

        bool IsRunning() override {
            return running_;
        }
    };
}


#endif //DBMIGRATION_HPP
