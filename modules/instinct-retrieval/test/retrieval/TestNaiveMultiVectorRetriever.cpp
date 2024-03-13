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
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;


    class MultiqueryRetrieverTest: public testing::Test {
    protected:
        void SetUp() override {
            auto root_path = std::filesystem::temp_directory_path() / "instinct-test " / ChronoUtils::GetCurrentTimestampString();
            std::filesystem::create_directories(root_path);

            llm_ = std::make_shared<OllamaChat>();

            size_t dimension = 4096;

            EmbeddingsPtr embedding_model = std::make_shared<OllamaEmbedding>();
            DuckDbVectoreStoreOptions db_options = {
                .table_name = "document_table",
                .db_file_path = root_path / "doc_store.db",
                .dimmension = dimension,
                .embeddings = embedding_model
            };
            doc_store_ = std::make_shared<DuckDBVectorStore>(db_options);

            DuckDbVectoreStoreOptions summary_db_options = {
                .table_name = "summaries_table",
                .db_file_path = root_path / "summary_store.db",
                .dimmension = dimension,
                .embeddings = embedding_model
            };
            summary_store_ = std::make_shared<DuckDBVectorStore>(summary_db_options);

            DuckDbVectoreStoreOptions query_db_options = {
                .table_name = "queries_table",
                .db_file_path = root_path / "query_store.db",
                .dimmension = dimension,
                .embeddings = embedding_model
            };
            hypothetical_quries_store_ = std::make_shared<DuckDBVectorStore>(query_db_options);

            asset_dir_ = std::filesystem::current_path() / "modules" / "instinct-retrieval" / "test" / "_assets";
        }

        std::filesystem::path asset_dir_;
        ChatModelPtr llm_;
        DocStorePtr doc_store_;
        VectorStorePtr summary_store_;
        VectorStorePtr hypothetical_quries_store_;
    };

    TEST_F(MultiqueryRetrieverTest, TestReteiveWithSummary) {
        auto retriever = CreateSummaryGuidedRetriever(
            llm_,
            doc_store_,
            summary_store_
        );

        // load all recipes in folder
        const auto ingestor = CreateDirectoryTreeIngestor(asset_dir_ / "docs" / "recipes");
        retriever->Ingest(ingestor->Load());

        const auto summary_count = summary_store_->CountDocuments();
        ASSERT_GT(summary_count, 0);

        // use simple search
        const auto doc_itr = retriever->Retrieve({.text = "Shortcake"});
        while (doc_itr->HasNext()) {
            std::cout << doc_itr->Next().text() << std::endl;
        }
    }

    TEST_F(MultiqueryRetrieverTest, TestReteiveWithHypotheticalQuries) {



    }



}
