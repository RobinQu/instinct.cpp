//
// Created by RobinQu on 2024/5/31.
//
#include <gtest/gtest.h>
#include "assistant/v2/tool/SimpleRetrieverOperator.hpp"
#include "database/duckdb/DuckDBConnectionPool.hpp"
#include "database/duckdb/DuckDBDataTemplate.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "store/duckdb/DuckDBVectorStoreOperator.hpp"
#include "AssistantTestGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_DATA_NS;
    class RetrieverOperatorTest: public BaseAssistantApiTest {
    protected:
        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
    };

    TEST_F(RetrieverOperatorTest, RetrieverLifecycle) {
        const auto retriever_operator = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "doc_table"});

        VectorStoreObject vector_store_object;
        vector_store_object.set_id("vs-1");
        vector_store_object.set_name("test-vs");
        vector_store_object.set_status(VectorStoreObject_VectorStoreStatus_completed);
        ASSERT_TRUE(retriever_operator->ProvisionRetriever(vector_store_object.id()));
        ASSERT_TRUE(retriever_operator->GetStatefulRetriever(vector_store_object.id()));
        ASSERT_TRUE(retriever_operator->GetStatelessRetriever(vector_store_object.id()));
        ASSERT_TRUE(retriever_operator->CleanupRetriever(vector_store_object.id()));
    }
}
