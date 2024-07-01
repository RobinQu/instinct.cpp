//
// Created by RobinQu on 2024/5/31.
//
#include <gtest/gtest.h>

#include <instinct/database/db_utils.hpp>
#include <instinct/database/duckdb/duckdb_connection_pool.hpp>
#include <instinct/database/duckdb/duckdb_data_template.hpp>
#include <instinct/embedding_model/openai_embedding.hpp>
#include <instinct/store/duckdb/duckdb_vector_store_operator.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    class DuckDBVectorStoreOperatorTest: public testing::Test {
    protected:
        DuckDBPtr duck_db_ = std::make_shared<duckdb::DuckDB>(nullptr);
        DuckDBConnectionPoolPtr connection_pool_ = CreateDuckDBConnectionPool(duck_db_);
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel();
        VectorStoreMetadataDataMapperPtr vector_store_metadata_data_mapper_ = std::make_shared<VectorStoreMetadataDataMapper>(CreateDuckDBDataMapper<VectorStoreInstanceMetadata, std::string>(connection_pool_));

        void SetUp() override {
            SetupLogging();
            duckdb::Connection conn(*duck_db_);
            conn.Query(R"(CREATE TABLE IF NOT EXISTS instinct_vector_store_metadata (
    instance_id VARCHAR PRIMARY KEY,
    metadata_schema VARCHAR NOT NULL,
    created_at TIMESTAMP DEFAULT now() NOT NULL,
    modified_at TIMESTAMP DEFAULT now() NOT NULL,
    embedding_table_name VARCHAR NOT NULL,
    custom VARCHAR
);)");
        }
    };

    TEST_F(DuckDBVectorStoreOperatorTest, VDBLifecycle) {
        const auto vdb_operator = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        const auto obj1 = vdb_operator->CreateInstance("vs-1");
        ASSERT_TRUE(obj1);

        const auto obj2 = vdb_operator->LoadInstance("vs-1");
        ASSERT_TRUE(obj2);

        const auto list1= vdb_operator->ListInstances();
        ASSERT_EQ(list1.size(), 1);
        ASSERT_EQ(list1[0], "vs-1");

        ASSERT_TRUE(vdb_operator->RemoveInstance("vs-1"));

        const auto list2 = vdb_operator->ListInstances();
        ASSERT_EQ(list2.size(), 0);
    }
}