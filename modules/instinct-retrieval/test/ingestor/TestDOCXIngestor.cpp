//
// Created by RobinQu on 2024/3/26.
//

#include <gtest/gtest.h>
#include <instinct/tools/metadata_schema_builder.hpp>

#include <instinct/ingestor/DOCXFileIngestor.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    class TestDOCXIngestor: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            corpus_dir = std::filesystem::current_path() / "_corpus";
        }
        std::filesystem::path corpus_dir;
    };

    TEST_F(TestDOCXIngestor, LoadDocuments) {
        const auto schema = CreateDocStorePresetMetadataSchema();
        DOCXFileIngestor ingestor { corpus_dir / "word/sample2.docx" };
        auto doc_itr = ingestor.Load()
        | rpp::operators::tap([&](const Document& doc) {
            ASSERT_EQ(doc.metadata_size(), schema->fields_size());
            LOG_INFO("doc = {}", doc.DebugString());
            ASSERT_FALSE(doc.text().empty());
        });
        auto docs = CollectVector(doc_itr.as_dynamic());
        // ASSERT_EQ(docs.size(), 15);
    }
}