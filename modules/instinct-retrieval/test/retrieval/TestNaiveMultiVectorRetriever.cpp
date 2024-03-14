//
// Created by RobinQu on 2024/3/13.
//


#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"
#include "embedding_model/OllamaEmbedding.hpp"
#include "ingestor/BaseIngestor.hpp"
#include "ingestor/DirectoryTreeIngestor.hpp"
#include "retrieval/MultiQueryRetriever.hpp"
#include "retrieval/MultiVectorRetriever.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;


    class MultiVectorRetrieverTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();

            auto root_path = std::filesystem::temp_directory_path() / "instinct-test" / std::to_string(ChronoUtils::GetCurrentTimeMillis());
            std::filesystem::create_directories(root_path);

            std::cout << "MultiVectorRetrieverTest at " << root_path << std::endl;

            OllamaConfiguration ollama_configuration = {.model_name = "phi"};
            llm_ = std::make_shared<OllamaChat>();

            size_t dimension = 2560;

            EmbeddingsPtr embedding_model = std::make_shared<OllamaEmbedding>();
            DuckDBStoreOptions db_options = {
                .table_name = "document_table",
                .db_file_path = root_path / "doc_store.db"
            };
            doc_store_ = CreateDuckDBDocStore(db_options);

            DuckDBStoreOptions summary_db_options = {
                .table_name = "summaries_table",
                .db_file_path = root_path / "summary_store.db",
                .dimmension = dimension,
            };
            summary_store_ = CreateDuckDBVectorStore(embedding_model, summary_db_options);

            DuckDBStoreOptions query_db_options = {
                .table_name = "queries_table",
                .db_file_path = root_path / "query_store.db",
                .dimmension = dimension
            };
            hypothetical_quries_store_ = CreateDuckDBVectorStore(embedding_model, query_db_options);

            asset_dir_ = std::filesystem::current_path() / "modules" / "instinct-retrieval" / "test" / "_assets";
        }

        std::filesystem::path asset_dir_;
        ChatModelPtr llm_;
        DocStorePtr doc_store_;
        VectorStorePtr summary_store_;
        VectorStorePtr hypothetical_quries_store_;
    };

    TEST_F(MultiVectorRetrieverTest, TestReteiveWithSummary) {
        auto retriever = CreateSummaryGuidedRetriever(
            llm_,
            doc_store_,
            summary_store_
        );

        // load all recipes in folder
        const auto recipes_dir = asset_dir_ / "docs" / "recipes";
        std::cout << "reading recipes from " << recipes_dir << std::endl;
        const auto ingestor = CreateDirectoryTreeIngestor(recipes_dir);
        retriever->Ingest(ingestor->Load());

        const auto summary_count = summary_store_->CountDocuments();
        ASSERT_GT(summary_count, 0);

        // use simple search
        const auto doc_itr = retriever->Retrieve({.text = "Shortcake"});
        while (doc_itr->HasNext()) {
            std::cout << doc_itr->Next().text() << std::endl;
        }
    }

    TEST_F(MultiVectorRetrieverTest, TestReteiveWithHypotheticalQuries) {



    }



}
