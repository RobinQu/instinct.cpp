//
// Created by vscode on 5/4/24.
//

#ifndef INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP
#define INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP

#include "AssistantGlobals.hpp"
#include "assistant/v2/db/DBMigration.hpp"
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataTemplate.hpp"
#include "object_store/FileSystemObjectStore.hpp"
#include "task_scheduler/ThreadPoolTaskScheduler.hpp"
#include "assistant/v2/service/AssistantFacade.hpp"
#include "assistant/v2/task_handler/FileBatchObjectBackgroundTask.hpp"
#include "assistant/v2/task_handler/RunObjectTaskHandler.hpp"
#include "ioc/ManagedApplicationContext.hpp"
#include "server/httplib/HttpLibServer.hpp"
#include "store/VectorStoreMetadataDataMapper.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    using namespace INSTINCT_DATA_NS;
    using namespace INSTINCT_SERVER_NS;


    /**
     * Utility class to configure instances required for a complete assistant api service.
     * Implementation should create suitable objects according to configuration.
     * @tparam ConnectionImpl
     * @tparam QueryResultImpl
     * @tparam TaskPayload
     */
    template<typename ConnectionImpl, typename  QueryResultImpl, typename TaskPayload = std::string>
    class IApplicationContextFactory {
    public:
        struct ApplicationContext: ManagedApplicationContext {
            ConnectionPoolPtr<ConnectionImpl, QueryResultImpl> connection_pool;
            DataTemplatePtr<AssistantObject, std::string> assistant_data_mapper;
            DataTemplatePtr<ThreadObject, std::string> thread_data_mapper;
            DataTemplatePtr<MessageObject, std::string> message_data_mapper;
            DataTemplatePtr<FileObject, std::string> file_data_mapper;
            DataTemplatePtr<RunObject, std::string> run_data_mapper;
            VectorStoreMetadataDataMapperPtr vector_store_metadata_data_mapper;
            VectorStoreDataMapperPtr vector_store_data_mapper;
            VectorStoreFileDataMapperPtr vector_store_file_data_mapper;
            VectorStoreFileBatchDataMapperPtr vector_store_file_batch_data_mapper;

            DataTemplatePtr<RunStepObject, std::string> run_step_data_mapper;
            ObjectStorePtr object_store;
            TaskSchedulerPtr<TaskPayload> task_scheduler;
            AssistantFacade assistant_facade;
            HttpLibServerPtr http_server;
            TaskHandlerPtr<TaskPayload> run_object_task_handler;
            RetrieverOperatorPtr retriever_operator;
            VectorStoreOperatorPtr vector_store_operator;
            TaskHandlerPtr<TaskPayload> file_object_task_handler;
            BackgroundTaskPtr file_batch_background_task;
            std::shared_ptr<DBMigration<ConnectionImpl, QueryResultImpl>> db_migration;
        };

        IApplicationContextFactory() = default;
        virtual ~IApplicationContextFactory()=default;
        IApplicationContextFactory(IApplicationContextFactory&&)=delete;
        IApplicationContextFactory(const IApplicationContextFactory&)=delete;
        virtual ApplicationContext& GetInstance() = 0;
    };

}


#endif //INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP
