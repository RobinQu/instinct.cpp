//
// Created by vscode on 5/4/24.
//

#ifndef INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP
#define INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP

#include <instinct/assistant_global.hpp>
#include <instinct/assistant/v2/db/db_migration.hpp>
#include <instinct/database/duckdb/duckdb_connection_pool.hpp>
#include <instinct/database/duckdb/duckdb_data_template.hpp>
#include <instinct/object_store/file_system_object_store.hpp>
#include <instinct/task_scheduler/thread_pool_task_scheduler.hpp>
#include <instinct/assistant/v2/service/assistant_facade.hpp>
#include <instinct/assistant/v2/task_handler/file_batch_object_background_task.hpp>
#include <instinct/assistant/v2/task_handler/run_object_task_handler.hpp>
#include <instinct/ioc/application_context.hpp>
#include <instinct/server/httplib/http_lib_server.hpp>
#include <instinct/store/vector_store_metadata_data_mapper.hpp>

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
    class ApplicationContext final: public ManagedApplicationContext {
    public:
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

}


#endif //INSTINCT_IAPPLICAITONCONTEXTFACTORY_HPP
