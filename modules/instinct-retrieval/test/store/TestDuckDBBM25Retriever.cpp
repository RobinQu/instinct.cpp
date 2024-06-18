//
// Created by RobinQu on 2024/6/18.
//
#include <gtest/gtest.h>

#include "ingestor/BaseIngestor.hpp"
#include "ingestor/ParquetFileIngestor.hpp"
#include "retrieval/duckdb/DuckDBBM25Retriever.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class TestDuckDBBM25Retriever: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            LOG_INFO("db_file_path={}", db_file_path);
        }
        std::filesystem::path db_file_path = std::filesystem::temp_directory_path() / (StringUtils::GenerateUUIDString() + ".db");
        DocStorePtr doc_store_ = CreateDuckDBDocStore({
            .table_name = "doc_table",
            .db_file_path = db_file_path
        });
        IngestorPtr ingestor_ = CreateParquetIngestor(std::filesystem::current_path( ) / "_corpus" / "huggingface_doc_train.parquet", "0:text,1:metadata:file_source:varchar", {.limit = 100});
    };

    TEST_F(TestDuckDBBM25Retriever, IndexLifecycle) {
        const auto retriever = CreateDuckDBBM25Retriever(doc_store_);
        if (const auto ptr = std::dynamic_pointer_cast<DuckDBBM25Retriever>(retriever)) {
            ptr->BuildIndex();
            ptr->DropIndex();
        }

    }

    TEST_F(TestDuckDBBM25Retriever, KeywordSearch) {
        const auto retriever = CreateDuckDBBM25Retriever(doc_store_, {.auto_build = true});
        retriever->Ingest(ingestor_->Load());
        auto doc_itr = retriever->Retrieve({.text = "metrics used in the Machine Learning community"});

        const std::vector<Document> result_set = CollectVector(doc_itr);
        for (const auto& doc: result_set) {
            std::cout << doc.ShortDebugString() << std::endl;
        }
        ASSERT_TRUE(!result_set.empty());

    }
}