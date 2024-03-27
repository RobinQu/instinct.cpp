//
// Created by RobinQu on 2024/3/16.
//
#include <gtest/gtest.h>
#include "store/duckdb/DuckDBDocStore.hpp"
#include "RetrievalTestGlobals.hpp"
#include "RetrievalGlobals.hpp"
#include "../../../instinct-core/include/tools/DocumentUtils.hpp"
#include "../../../instinct-core/include/tools/MetadataSchemaBuilder.hpp"
#include "tools/TensorUtils.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class DuckDBDocStoreTest : public ::testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(DuckDBDocStoreTest, CRUDWithoutSchema) {
        auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = instinct::test::ensure_random_temp_folder() /
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
            ASSERT_TRUE(!update_result2.returned_ids(i).empty());
        }

        LOG_INFO("count documents");
        xn = doc_store->CountDocuments();
        ASSERT_EQ(xn, n-m+j+1);
    }

    TEST_F(DuckDBDocStoreTest, CRUDWithSchema) {
        auto schema_builder = MetadataSchemaBuilder::Create();
        schema_builder->DefineString("genus");
        schema_builder->DefineInt32("age");
        schema_builder->DefineInt32("sex");
        auto schema = schema_builder->Build();
        auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = instinct::test::ensure_random_temp_folder() /
                                                                      "doc_store_with_out_schema.db",
                                              }, schema);

        LOG_INFO("schema = {}", schema->DebugString());

        ASSERT_THROW({ // insert with document lacking some metadata
            Document document;
            document.set_text("trouble");
            doc_store->AddDocument(document);
        }, InstinctException);

        ASSERT_NO_THROW({ // insert with document containing extra metadata, which is permitted by default. This can be toggled with `DuckDBStoreOptions::bypass_unknown_fields` option.
            Document document;
            document.set_text("Zebras are African equines with distinctive black-and-white striped coats. There are three living species: Grévy's zebra, the plains zebra, and the mountain zebra. Zebras share the genus Equus with horses and asses, the three groups being the only living members of the family Equidae. ");
            DocumentMetadataMutator metadata_mutator  {&document};
            metadata_mutator.SetString("genus", "Equus");
            metadata_mutator.SetInt32("age", 8);
            metadata_mutator.SetInt32("sex", 1);
            metadata_mutator.SetString("origin", "North America");

            doc_store->AddDocument(document);
            LOG_INFO("returned doc id: {}", document.id());

            // lookup
            ASSERT_TRUE(!document.id().empty());
            doc_store->MultiGetDocuments({document.id()})
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const Document& doc) {
                    auto violations = DocumentUtils::ValidateDocument(doc, schema);
                    for(const auto& violation: violations) {
                        LOG_INFO("field={}, msg={}", violation.field_name, violation.message);
                    }
                    ASSERT_TRUE(violations.empty());
                })
            ;
        });


    }


}
