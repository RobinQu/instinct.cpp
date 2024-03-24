//
// Created by RobinQu on 2024/3/13.
//


#include <gtest/gtest.h>

#include "LLMTestGlobals.hpp"
#include "chat_model/OllamaChat.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"
#include "retrieval/MultiQueryRetriever.hpp"
#include "retrieval/MultiVectorRetriever.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"



namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;


    class MultiVectorRetrieverTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            auto root_path = instinct::test::ensure_random_temp_folder();
            std::cout << "MultiVectorRetrieverTest at " << root_path << std::endl;
            llm_ = instinct::test::create_pesudo_chat_model();
//            llm_ = instinct::test::create_local_chat_model();

            size_t dimension = 4096;

            auto schema_builder = MetadataSchemaBuilder::Create();
            schema_builder->DefineString("parent_doc_id");
            auto meta_schema = schema_builder->Build();

            EmbeddingsPtr embedding_model = instinct::test::create_pesudo_embedding_model(dimension);
            DuckDBStoreOptions db_options = {
                .table_name = "document_table",
                .db_file_path = root_path / "doc_store.db"
            };
            doc_store_ = CreateDuckDBDocStore(db_options);

            DuckDBStoreOptions summary_db_options = {
                .table_name = "summaries_table",
                .db_file_path = root_path / "summary_store.db",
                .dimension = dimension,
            };
            summary_store_ = CreateDuckDBVectorStore(embedding_model, summary_db_options, meta_schema);

            DuckDBStoreOptions query_db_options = {
                .table_name = "queries_table",
                .db_file_path = root_path / "query_store.db",
                .dimension = dimension
            };
            hypothetical_queries_store_ = CreateDuckDBVectorStore(embedding_model, query_db_options, meta_schema);

            asset_dir_ = std::filesystem::current_path() / "modules" / "instinct-retrieval" / "test" / "_assets";

            // load all recipes in folder
            const auto recipes_dir = asset_dir_ / "docs" / "recipes";
            std::cout << "reading recipes from " << recipes_dir << std::endl;
            recipes_ingestor_ = CreateDirectoryTreeIngestor(recipes_dir);
        }

        std::filesystem::path asset_dir_;
        ChatModelPtr llm_;
        DocStorePtr doc_store_;
        VectorStorePtr summary_store_;
        VectorStorePtr hypothetical_queries_store_;
        IngestorPtr recipes_ingestor_;
    };

    TEST_F(MultiVectorRetrieverTest, TestReteiveWithSummary) {
        auto retriever = CreateSummaryGuidedRetriever(
            llm_,
            doc_store_,
            summary_store_,
            nullptr,
            {.parent_doc_id_key = "parent_doc_id"}
        );
        retriever->Ingest(recipes_ingestor_->Load());

        const auto summary_count = summary_store_->CountDocuments();
        ASSERT_GT(summary_count, 0);

        // use simple search
        const auto doc_itr = retriever->Retrieve({.text = "Shortcake", .top_k = 2});

        auto doc_vec = CollectVector(doc_itr);
        ASSERT_EQ(doc_vec.size(), 2);
    }

    TEST_F(MultiVectorRetrieverTest, TestRetrieverWithHypotheticalQueries) {
        auto retriever = CreateHypotheticalQueriesGuidedRetriever(
                llm_,
                doc_store_,
                hypothetical_queries_store_,
                nullptr,
                {.parent_doc_id_key = "parent_doc_id"}
        );
        retriever->Ingest(recipes_ingestor_->Load());

        const auto queries_count = hypothetical_queries_store_->CountDocuments();
        ASSERT_GT(queries_count, 0);

        // simple search
        const auto doc_itr = retriever->Retrieve({.text = "Shortcake", .top_k = 2});

        auto doc_vec = CollectVector(doc_itr);
        // first recall may contain duplicated docs which can be filtered away
        ASSERT_TRUE(doc_vec.size() <= 2);
    }



}
