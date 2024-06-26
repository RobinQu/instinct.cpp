//
// Created by RobinQu on 2024/4/3.
//
#include <gtest/gtest.h>

#include <instinct/RetrievalGlobals.hpp>
#include <instinct/RetrievalTestGlobals.hpp>
#include <instinct/ingestor/ParquetFileIngestor.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    class ParquetFileIngestorTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
        std::filesystem::path asset_dir_ = std::filesystem::current_path() / "_corpus";
    };

    TEST_F(ParquetFileIngestorTest, TestRemoteURL) {
        // https://huggingface.co/datasets/m-ric/huggingface_doc_qa_eval
        const auto ingestor = CreateParquetIngestor(asset_dir_ / "huggingface_doc_qa_eval.parquet", "1:t,0:m:context:varchar,2:m:answer:varchar");
        const auto records = CollectVector(ingestor->Load());
        ASSERT_EQ(records.size(), 67);
        for (auto& record: records) {
            ASSERT_TRUE(!record.text().empty());
            // preset metadata are filled in
            ASSERT_EQ(record.metadata_size(), 7);
            ASSERT_TRUE(DocumentUtils::HasMetadataField(record, "context"));
            ASSERT_TRUE(DocumentUtils::HasMetadataField(record, "answer"));
            ASSERT_TRUE(!record.metadata(0).string_value().empty());
        }
    }

    TEST_F(ParquetFileIngestorTest, TestLimit) {
        const auto ingestor = CreateParquetIngestor(asset_dir_ / "huggingface_doc_qa_eval.parquet", "0:text,1:metadata:file_source:varchar", {.limit=5});
        const auto records = CollectVector(ingestor->Load());
        ASSERT_EQ(records.size(), 5);
    }
}

