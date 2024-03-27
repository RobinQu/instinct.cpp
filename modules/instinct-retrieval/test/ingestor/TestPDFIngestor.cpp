//
// Created by RobinQu on 2024/3/26.
//
#include <gtest/gtest.h>

#include "ingestor/PDFFileIngestor.hpp"


namespace INSTINCT_RETRIEVAL_NS {

    class TestPDFIngestor: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            corpus_dir = std::filesystem::current_path() / "_corpus";
        }


        std::filesystem::path corpus_dir;
    };


    TEST_F(TestPDFIngestor, LoadDocuments) {
        PDFFileIngestor ingestor { corpus_dir / "papers/attention_is_all_you_need.pdf"};
        auto doc_itr = ingestor.Load()
        | rpp::operators::tap([](const Document& doc) {
            ASSERT_EQ(doc.metadata_size(), 2);// expecting page_no and source
            LOG_INFO("doc = {}", doc.DebugString());
            ASSERT_FALSE(doc.text().empty());
        });
        auto docs = CollectVector(doc_itr.as_dynamic());
        ASSERT_EQ(docs.size(), 15);
    }

}