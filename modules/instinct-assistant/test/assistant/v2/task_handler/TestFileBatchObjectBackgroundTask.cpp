//
// Created by RobinQu on 2024/6/10.
//
#include <gtest/gtest.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/task_handler/FileBatchObjectBackgroundTask.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    class TestFileBatchObjectBackgroundTask: public BaseAssistantApiTest {
    protected:
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        RetrieverOperatorPtr retriever_operator_ = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "docs_" + ChronoUtils::GetCurrentTimestampString()});
        VectorStoreServicePtr vector_store_service_ = CreateVectorStoreService(nullptr, retriever_operator_);
    };

    TEST_F(TestFileBatchObjectBackgroundTask, Lifecycle) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};
        file_batch_object_background_task.Start();
        file_batch_object_background_task.Shutdown();
    }

    TEST_F(TestFileBatchObjectBackgroundTask, Handle) {
        FileBatchObjectBackgroundTask file_batch_object_background_task {vector_store_service_, retriever_operator_};
        file_batch_object_background_task.Handle();

    }
}