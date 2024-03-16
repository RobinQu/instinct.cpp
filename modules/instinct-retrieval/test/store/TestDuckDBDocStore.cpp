//
// Created by RobinQu on 2024/3/16.
//
#include <gtest/gtest.h>
#include "store/duckdb/DuckDBDocStore.hpp"
#include "RetrievalTestGlobals.hpp"
#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    class DuckDBDocStoreTest : public ::testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(DuckDBDocStoreTest, CRUDWithoutSchema) {
        auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = test::ensure_random_temp_folder() /
                                                                      "doc_store_with_out_schema.db",
                                              });

        LOG_INFO("insert one");
        Document doc1;
        doc1.set_text("doc1");
        doc_store->AddDocument(doc1);
        ASSERT_TRUE(!doc1.id().empty());

        LOG_INFO("insert with docs");
        std::vector<Document> docs;
        int n=10;
        for(int i=0;i<n;i++)  {
            Document doc;
            doc.set_text("doc of no " + std::to_string(i));
            docs.push_back(doc);
        }
        UpdateResult updateResult;
        doc_store->AddDocuments(docs, updateResult);
        ASSERT_TRUE(updateResult.failed_documents().empty());
        ASSERT_EQ(updateResult.affected_rows(), n);
        ASSERT_EQ(updateResult.returned_ids_size(), n);
        for (int i = 0; i < n; ++i) {
            ASSERT_EQ(docs[i].id(), updateResult.returned_ids(i));
        }

        LOG_INFO("mget");
        const auto mget_itr = doc_store->MultiGetDocuments({updateResult.returned_ids().begin(), updateResult.returned_ids().end()});
        const auto mget_vector = CollectVector(mget_itr);
        ASSERT_EQ(mget_vector.size(), n);
        for (int i = 0; i < n; ++i) {
            ASSERT_EQ(mget_vector[i].id(), docs[i].id());
            ASSERT_EQ(mget_vector[i].text(), docs[i].text());
        }

        size_t xn = doc_store->CountDocuments();
        ASSERT_EQ(xn, n+1);

        LOG_INFO("delete docs");
        int m = 2;
        std::vector<std::string> deleted_ids;
        deleted_ids.reserve(m);
        for(int i=0;i<m;i++) {
            deleted_ids.push_back(docs[i].id());
        }
        UpdateResult delete_result;
        doc_store->DeleteDocuments(deleted_ids, delete_result);
        ASSERT_EQ(deleted_ids.size(), delete_result.affected_rows());
        //ASSERT_EQ(deleted_ids, delete_result.returned_ids());
        for (int i = 0; i < m; ++i) {
            ASSERT_EQ(deleted_ids[i], delete_result.returned_ids(i));
        }

        xn = doc_store->CountDocuments();
        ASSERT_EQ(xn, n-m+1);

        LOG_INFO("insert with second batch of docs using AsyncIterator");
        int j=10;
        std::vector<Document> docs2;
        for(int i=0;i<j;i++)  {
            Document doc;
            doc.set_text("batch2, doc of no " + std::to_string(i));
            docs2.push_back(doc);
        }
        UpdateResult update_result2;
        auto doc_source = rpp::source::from_iterable(docs2);
        doc_store->AddDocuments(doc_source, update_result2);
        ASSERT_TRUE(update_result2.failed_documents().empty());
        ASSERT_EQ(update_result2.affected_rows(), j);
        ASSERT_EQ(update_result2.returned_ids_size(), j);
        for (int i = 0; i < j; ++i) {
//            ASSERT_EQ(docs2[i].id(), update_result2.returned_ids(i));
            ASSERT_TRUE(!update_result2.returned_ids(i).empty());
        }

        LOG_INFO("count documents");
        xn = doc_store->CountDocuments();
        ASSERT_EQ(xn, n-m+j+1);
    }

    TEST_F(DuckDBDocStoreTest, CRUDWithSchema) {

    }


}
