//
// Created by RobinQu on 2024/4/9.
//

#ifndef AGENTTESTGLOBALS_HPP
#define AGENTTESTGLOBALS_HPP

#include <cmrc/cmrc.hpp>

#include "assistant/v2/data_mapper/VectorStoreDataMapper.hpp"
#include "assistant/v2/data_mapper/VectorStoreFileBatchDataMapper.hpp"
#include "assistant/v2/data_mapper/VectorStoreFileDataMapper.hpp"
#include "assistant/v2/service/impl/VectorStoreServiceImpl.hpp"
#include "store/VectorStoreMetadataDataMapper.hpp"
#include "store/duckdb/DuckDBVectorStoreOperator.hpp"
CMRC_DECLARE(instinct::assistant);

#include <gtest/gtest.h>
#include "AssistantGlobals.hpp"
#include "assistant/v2/service/IFileService.hpp"
#include "assistant/v2/service/IRunService.hpp"
#include "assistant/v2/service/IThreadService.hpp"
#include "assistant/v2/service/impl/AssistantServiceImpl.hpp"
#include "assistant/v2/service/impl/FileServiceImpl.hpp"
#include "assistant/v2/service/impl/MessageServiceImpl.hpp"
#include "assistant/v2/service/impl/RunServiceImpl.hpp"
#include "assistant/v2/service/impl/ThreadServiceImpl.hpp"
#include "database/DBUtils.hpp"
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataTemplate.hpp"
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
            auto sql_file = embedded_fs.open("db_migration/001/up.sql");
            const auto sql_line = std::string {sql_file.begin(), sql_file.end()};
            const auto status = DBUtils::ExecuteSQL(sql_line, connection_pool_);
            assert_query_ok(status);
            LOG_INFO("database is initialized at {}", db_file_path);
        }

        cmrc::embedded_filesystem embedded_fs = cmrc::instinct::assistant::get_filesystem();

        std::filesystem::path db_file_path = std::filesystem::temp_directory_path() / fmt::format("assistant_test_{}.db", ChronoUtils::GetCurrentTimeMillis());

        DuckDBPtr duck_db_ = std::make_shared<DuckDB>(db_file_path);

        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);

        DataTemplatePtr<AssistantObject, std::string> assistant_data_mapper = CreateDuckDBDataMapper<AssistantObject, std::string>(connection_pool_);

        DataTemplatePtr<ThreadObject, std::string> thread_data_mapper = CreateDuckDBDataMapper<ThreadObject, std::string>(connection_pool_);

        DataTemplatePtr<MessageObject, std::string> message_data_mapper = CreateDuckDBDataMapper<MessageObject, std::string>(connection_pool_);

        DataTemplatePtr<FileObject, std::string> file_data_mapper = CreateDuckDBDataMapper<FileObject, std::string>(connection_pool_);

        DataTemplatePtr<RunObject, std::string> run_data_mapper = CreateDuckDBDataMapper<RunObject, std::string>(connection_pool_);

        DataTemplatePtr<RunStepObject, std::string> run_step_data_mapper = CreateDuckDBDataMapper<RunStepObject, std::string>(connection_pool_);

        VectorStoreDataMapperPtr vector_store_data_mapper = std::make_shared<VectorStoreDataMapper>(CreateDuckDBDataMapper<VectorStoreObject, std::string>(connection_pool_));

        VectorStoreFileDataMapperPtr vector_store_file_data_mapper = std::make_shared<VectorStoreFileDataMapper>(CreateDuckDBDataMapper<VectorStoreFileObject, std::string>(connection_pool_));

        VectorStoreFileBatchDataMapperPtr vector_store_file_batch_data_mapper = std::make_shared<VectorStoreFileBatchDataMapper>(CreateDuckDBDataMapper<VectorStoreFileBatchObject, std::string>(connection_pool_));

        VectorStoreMetadataDataMapperPtr vector_store_metadata_data_mapper = std::make_shared<VectorStoreMetadataDataMapper>(CreateDuckDBDataMapper<VectorStoreInstanceMetadata, std::string>(connection_pool_));

        // std::filesystem::path migration_dir = std::filesystem::current_path() / "_assets" / "db_migration";

        ObjectStorePtr filesystem_object_store_ = std::make_shared<FileSystemObjectStore>(std::filesystem::temp_directory_path() / "assistant_api_test");

        // no queue is assigned here, so it will create a in-memory queue by default
        CommonTaskSchedulerPtr task_scheduler_ = CreateThreadPoolTaskScheduler();

        AssistantServicePtr CreateAssistantService() {
            return std::make_shared<AssistantServiceImpl>(assistant_data_mapper);
        }

        FileServicePtr CreateFileService() {
            return std::make_shared<FileServiceImpl>(file_data_mapper, filesystem_object_store_);
        }

        RunServicePtr CreateRunServiceWithoutScheduler() {
            return std::make_shared<RunServiceImpl>(thread_data_mapper, run_data_mapper, run_step_data_mapper, message_data_mapper, nullptr);
        }

        ThreadServicePtr CreateThreadService() {
            return std::make_shared<ThreadServiceImpl>(thread_data_mapper, message_data_mapper, run_data_mapper, run_step_data_mapper);
        }

        MessageServicePtr CreateMessageService() {
            return std::make_shared<MessageServiceImpl>(message_data_mapper);
        }

        VectorStoreServicePtr CreateVectorStoreService() {
            return std::make_shared<VectorStoreServiceImpl>(
                vector_store_file_data_mapper,
                vector_store_data_mapper,
                vector_store_file_batch_data_mapper,
                nullptr,
                nullptr
                );
        }

    };








}


#endif //AGENTTESTGLOBALS_HPP
