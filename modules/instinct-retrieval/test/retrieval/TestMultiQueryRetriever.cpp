//
// Created by RobinQu on 2024/3/13.
//
#include <gtest/gtest.h>
#include "retrieval/MultiQueryRetriever.hpp"
#include "RetrievalTestGlobals.hpp"
#include "store/duckdb/DuckDBStoreInternal.hpp"
#include "retrieval/VectorStoreRetriever.hpp"
#include "retrieval/IRetriever.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;


    class MultiQueryRetrieverTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();

            auto root_path = instinct::test::ensure_random_temp_folder();
            // corpus data is copied to build tree in CMakeLists.txt
            asset_dir_ = std::filesystem::current_path() / "_corpus";

            std::cout << "MultiQueryRetrieverTest at " << root_path << std::endl;

            llm_ = instinct::test::create_pesudo_chat_model();

            size_t dimension = 4096;

            auto schema_builder = MetadataSchemaBuilder::Create();
//            schema_builder->DefineString("parent_doc_id");
            auto meta_schema = schema_builder->Build();

            EmbeddingsPtr embedding_model = instinct::test::create_pesudo_embedding_model(dimension);

            DuckDBStoreOptions vector_db_options = {
                    .table_name = "doc_table",
                    .db_file_path = root_path / "base_store.db",
                    .dimension = dimension,
            };
            auto vector_store = CreateDuckDBVectorStore(embedding_model, vector_db_options, meta_schema);

            base_retriever_ = CreateVectorStoreRetriever(vector_store);

            const auto recipes_dir = asset_dir_ / "recipes";
            std::cout << "reading recipes from " << recipes_dir << std::endl;
            recipes_ingestor_ = CreateDirectoryTreeIngestor(recipes_dir);
        }

        std::filesystem::path asset_dir_;
        ChatModelPtr llm_;
        VectorStorePtr vector_store_ptr_;
        StatefulRetrieverPtr base_retriever_;
        IngestorPtr recipes_ingestor_;
    };

    TEST_F(MultiQueryRetrieverTest, SimpleTest) {
        auto retriever = std::dynamic_pointer_cast<BaseRetriever>(base_retriever_);
        auto multi_query_retriever = CreateMultiQueryRetriever(
                retriever,
                llm_
        );

        base_retriever_->Ingest(recipes_ingestor_->Load());

        auto result_itr = multi_query_retriever->Retrieve({.text = "How to prepare dinner?"});
        auto result_vec = CollectVector(result_itr);
        for(const auto& doc: result_vec) {
            std::cout << doc.DebugString() << std::endl;
        }
        ASSERT_TRUE(!result_vec.empty());
    }
}

