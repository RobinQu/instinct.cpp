//
// Created by RobinQu on 2024/3/16.
//
#include <gtest/gtest.h>
#include <instinct/store/duckdb/DuckDBDocStore.hpp>
#include <instinct/RetrievalTestGlobals.hpp>
#include <instinct/RetrievalGlobals.hpp>
#include <instinct/tools/DocumentUtils.hpp>
#include <instinct/tools/MetadataSchemaBuilder.hpp>
#include <instinct/tools/TensorUtils.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    struct Animal {
        std::string name;
        std::string genus;
        int age;
        int sex;
        std::string origin;
        std::string description;
    };

    static std::string insert_animal(const DocStorePtr& doc_store, const Animal& animal) {
        Document document;
        document.set_text(animal.description);
        DocumentUtils::SetStringValueMetadataFiled(document, "name", animal.name);
        DocumentUtils::SetStringValueMetadataFiled(document, "genus", animal.genus);
        DocumentUtils::SetIntValueMetadataFiled(document, "age", animal.age);
        DocumentUtils::SetIntValueMetadataFiled(document, "sex", animal.sex);
        DocumentUtils::SetStringValueMetadataFiled(document, "origin", animal.origin);
        doc_store->AddDocument(document);
        LOG_INFO("returned doc id: {}", document.id());
        return document.id();
    }

    class DuckDBDocStoreTest : public ::testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }


    };

    TEST_F(DuckDBDocStoreTest, CRUDWithoutSchema) {
        auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = INSTINCT_LLM_NS::ensure_random_temp_folder() /
                                                                      "doc_store_with_out_schema.db",
                                              });

        LOG_INFO("insert one");
        Document doc1;
        doc1.set_text("doc1");
        DocumentUtils::AddMissingPresetMetadataFields(doc1);
        doc_store->AddDocument(doc1);
        ASSERT_TRUE(!doc1.id().empty());

        LOG_INFO("insert with docs");
        std::vector<Document> docs;
        int n=10;
        for(int i=0;i<n;i++)  {
            Document doc;
            doc.set_text("doc of no " + std::to_string(i));
            DocumentUtils::AddMissingPresetMetadataFields(doc);
            docs.push_back(doc);
        }
        UpdateResult updateResult;
        doc_store->AddDocuments(docs, updateResult);
        ASSERT_TRUE(updateResult.failed_documents().empty());
        ASSERT_EQ(updateResult.affected_rows(), n);
        ASSERT_EQ(updateResult.returned_ids_size(), n);


        LOG_INFO("mget");
        const auto mget_itr = doc_store->MultiGetDocuments({updateResult.returned_ids().begin(), updateResult.returned_ids().end()});
        const auto mget_vector = CollectVector(mget_itr);
        ASSERT_EQ(mget_vector.size(), n);
        for (int i = 0; i < n; ++i) {
            ASSERT_EQ(mget_vector[i].id(), updateResult.returned_ids(i));
            ASSERT_EQ(mget_vector[i].text(), docs[i].text());
        }

        size_t xn = doc_store->CountDocuments();
        ASSERT_EQ(xn, n+1);

        LOG_INFO("delete docs");
        int m = 2;
        std::vector<std::string> deleted_ids;
        deleted_ids.reserve(m);
        for(int i=0;i<m;i++) {
            deleted_ids.push_back(updateResult.returned_ids(i));
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
            DocumentUtils::AddMissingPresetMetadataFields(doc);
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
        schema_builder->DefineString("name");
        schema_builder->DefineString("genus");
        schema_builder->DefineInt32("age");
        schema_builder->DefineInt32("sex");
        schema_builder->DefineString("origin");
        auto schema = schema_builder->Build();
        auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = INSTINCT_LLM_NS::ensure_random_temp_folder() /
                                                                      "doc_store_with_out_schema.db",
                                              }, schema);

        LOG_INFO("schema = {}", schema->DebugString());

        ASSERT_THROW({ // insert with document lacking some metadata
            Document document;
            document.set_text("trouble");
            doc_store->AddDocument(document);
        }, InstinctException);

        ASSERT_NO_THROW({ // insert with document containing extra metadata, which is permitted by default. This can be toggled with `DuckDBStoreOptions::bypass_unknown_fields` option.
            const auto id = insert_animal(doc_store, {
                .name = "Zebra",
                .genus = "Equus",
                .age = 8,
                .sex = 1,
                .origin = "North America",
                .description = "Zebras are African equines with distinctive black-and-white striped coats. There are three living species: Grévy's zebra, the plains zebra, and the mountain zebra. Zebras share the genus Equus with horses and asses, the three groups being the only living members of the family Equidae. "

            });

            // lookup
            ASSERT_TRUE(!id.empty());
            doc_store->MultiGetDocuments({id})
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

    TEST_F(DuckDBDocStoreTest, DeleteWithFilter) {
        auto schema_builder = MetadataSchemaBuilder::Create();
        schema_builder->DefineString("name");
        schema_builder->DefineString("genus");
        schema_builder->DefineInt32("age");
        schema_builder->DefineInt32("sex");
        schema_builder->DefineString("origin");
        const auto schema = schema_builder->Build();
        const auto doc_store = CreateDuckDBDocStore({
                                                      .table_name = "animal_table",
                                                      .db_file_path = INSTINCT_LLM_NS::ensure_random_temp_folder() /
                                                                      "doc_store_with_out_schema.db"
                                              },
                                              schema);

        insert_animal(doc_store, {
                .genus = "Equus",
                .age = 8,
                .sex = 1,
                .origin = "North America",
                .description = "Zebras are African equines with distinctive black-and-white striped coats. There are three living species: Grévy's zebra, the plains zebra, and the mountain zebra. Zebras share the genus Equus with horses and asses, the three groups being the only living members of the family Equidae. "

            });
        insert_animal(doc_store, {
                .name = "Lion",
                .genus = "Panthera",
                .age = 6,
                .sex = 1,
                .origin = "African",
                .description = "The lion (Panthera leo) is a large cat of the genus Panthera, native to Africa and India. It has a muscular, broad-chested body; a short, rounded head; round ears; and a hairy tuft at the end of its tail. It is sexually dimorphic; adult male lions are larger than females and have a prominent mane. It is a social species, forming groups called prides. A lion's pride consists of a few adult males, related females, and cubs. Groups of female lions usually hunt together, preying mostly on large ungulates. The lion is an apex and keystone predator; although some lions scavenge when opportunities occur and have been known to hunt humans, lions typically do not actively seek out and prey on humans."
            });
        insert_animal(doc_store, {
                .name = "Giraffe",
                .genus = "Giraffa",
                .age = 3,
                .sex = 0,
                .origin = "African",
                .description = "The giraffe is a large African hoofed mammal belonging to the genus Giraffa. It is the tallest living terrestrial animal and the largest ruminant on Earth. Traditionally, giraffes have been thought of as one species, Giraffa camelopardalis, with nine subspecies. Most recently, researchers proposed dividing them into up to eight extant species due to new research into their mitochondrial and nuclear DNA, and individual species can be distinguished by their fur coat patterns. Seven other extinct species of Giraffa are known from the fossil record."
            });

        ASSERT_EQ(doc_store->CountDocuments(), 3);

        SearchQuery search_query;
        search_query.mutable_term()->set_name("origin");
        search_query.mutable_term()->mutable_term()->set_string_value("North America");
        UpdateResult update_result;
        doc_store->DeleteDocuments(search_query, update_result);
        ASSERT_EQ(update_result.affected_rows(), 1);
        ASSERT_EQ(doc_store->CountDocuments(), 2);
    }


}
