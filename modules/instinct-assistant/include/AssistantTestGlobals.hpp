//
// Created by RobinQu on 2024/4/9.
//

#ifndef AGENTTESTGLOBALS_HPP
#define AGENTTESTGLOBALS_HPP


#include <gtest/gtest.h>
#include "AssistantGlobals.hpp"
#include "agent/state/InMemoryStateManager.hpp"
#include "agent/state/IStateManager.hpp"
#include "assistant/v2/service/impl/AssistantServiceImpl.hpp"
#include "database/DBUtils.hpp"
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataMapper.hpp"
#include "object_store/FileSystemObjectStore.hpp"
#include "object_store/IObjectStore.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"

namespace INSTINCT_ASSISTANT_NS {
    using namespace INSTINCT_DATA_NS;
    using namespace v2;

    class BaseAssistantApiTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            const auto status = DBUtils::ExecuteSQL(migration_dir / "001" / "up.sql", connection_pool_);
            assert_query_ok(status);
            LOG_INFO("database is intialized at {}", db_file_path);
        }

        std::filesystem::path db_file_path = std::filesystem::temp_directory_path() / fmt::format("assistant_test_{}.db", ChronoUtils::GetCurrentTimeMillis());

        DuckDBPtr duck_db_ = std::make_shared<DuckDB>(db_file_path);

        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);

        DataMapperPtr<AssistantObject, std::string> assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(connection_pool_);

        DataMapperPtr<ThreadObject, std::string> thread_data_mapper = CreateDuckDBDataMapper<ThreadObject, std::string>(connection_pool_);

        DataMapperPtr<MessageObject, std::string> message_data_mapper = CreateDuckDBDataMapper<MessageObject, std::string>(connection_pool_);

        DataMapperPtr<FileObject, std::string> file_data_mapper = CreateDuckDBDataMapper<FileObject, std::string>(connection_pool_);

        DataMapperPtr<RunObject, std::string> run_data_mapper = CreateDuckDBDataMapper<RunObject, std::string>(connection_pool_);

        DataMapperPtr<RunStepObject, std::string> run_step_data_mapper = CreateDuckDBDataMapper<RunStepObject, std::string>(connection_pool_);

        std::filesystem::path migration_dir = std::filesystem::current_path() / "_assets" / "db_migration";

        ObjectStorePtr filesystem_object_store_ = std::make_shared<FileSystemObjectStore>(std::filesystem::temp_directory_path() / "assistant_api_test");

        // no queue is assigned here, so it will create a in-memory queue by default
        CommonTaskSchedulerPtr task_scheduler_ = CreateThreadPoolTaskScheduler();

        StateManagerPtr state_manager_ = std::make_shared<InMemoryStateManager>();
    };





}


#endif //AGENTTESTGLOBALS_HPP
