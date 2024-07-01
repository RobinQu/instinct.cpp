//
// Created by RobinQu on 2024/3/6.
//
#include <gtest/gtest.h>
#include <random>
#include <ranges>
#include <instinct/retrieval_global.hpp>
#include <instinct/store/duckdb/duckdb_vector_store.hpp>
#include <instinct/tools/chrono_utils.hpp>
#include <instinct/retrieval_test_global.hpp>


namespace INSTINCT_RETRIEVAL_NS {



    class DuckDBVectorStoreTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            s1 = std::make_shared<MetadataSchema>();
            auto* name_field = s1->add_fields();
            name_field->set_name("name");
            name_field->set_type(VARCHAR);
            auto* address_field = s1->add_fields();
            address_field->set_name("address");
            address_field->set_type(VARCHAR);
            auto* age_field = s1->add_fields();
            age_field->set_name("age");
            age_field->set_type(INT32);
        }

        std::shared_ptr<MetadataSchema> s1;
    };

    TEST_F(DuckDBVectorStoreTest, make_create_table_sql) {
        const auto sql = details::make_create_table_sql("test_tb1", 128, s1);
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "CREATE TABLE IF NOT EXISTS test_tb1(id UUID PRIMARY KEY, text VARCHAR NOT NULL, vector FLOAT[128] NOT NULL, name VARCHAR, address VARCHAR, age INTEGER);");
    }

    TEST_F(DuckDBVectorStoreTest, make_search_sql) {
        SearchQuery mq;
        auto predicate = mq.mutable_term();
        predicate->set_name("address");
        predicate->mutable_term()->set_string_value("shanghai");
        auto sql = details::make_search_sql("tb1", s1, INSTINCT_LLM_NS::make_zero_vector(), mq);
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "SELECT id, text, name, address, age, array_cosine_similarity(vector, array_value(0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT)) AS similarity FROM tb1 WHERE address = 'shanghai' ORDER BY similarity DESC LIMIT 10;");

        sql = details::make_search_sql("tb1", s1, INSTINCT_LLM_NS::make_zero_vector(), SearchQuery {});
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "SELECT id, text, name, address, age, array_cosine_similarity(vector, array_value(0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT)) AS similarity FROM tb1 ORDER BY similarity DESC LIMIT 10;");

    }

    TEST_F(DuckDBVectorStoreTest, make_delete_sql) {
        auto sql = details::make_delete_sql("tb1", std::vector<std::string>{"1","2","3","4"});
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "DELETE FROM tb1 WHERE id IN ('1','2','3','4');");
    }

    TEST_F(DuckDBVectorStoreTest, TestSimpleRecall) {
        size_t dim = 128;
        auto db_file_path = INSTINCT_LLM_NS::ensure_random_temp_folder() / "test.db";
        LOG_INFO("db_file_path={}", db_file_path);
        auto embeddings = INSTINCT_LLM_NS::create_pesudo_embedding_model(dim);

        auto store = CreateDuckDBVectorStore(
            embeddings,
            { .table_name = "test_table_1", .db_file_path = db_file_path, .dimension = dim}
        );

        std::vector<Document> docs;
        for (int i: std::views::iota (0,1000)) {
            Document document;
            document.set_text(std::to_string(i));
            DocumentUtils::AddMissingPresetMetadataFields(document);
            docs.push_back(document);
        }
        UpdateResult update_result;
        store->AddDocuments(docs, update_result);

        int max = 100, n=0;
        for(const auto& [text, embedding]: embeddings->get_caches()) {
            if(++n==max) {
                break;
            }
            auto t1 = ChronoUtils::GetCurrentTimeMillis();
            SearchRequest search_request;
            search_request.set_query(text);
            search_request.set_top_k(5);
            auto itr = store->SearchDocuments(search_request);
            std::cout << "search done in " << std::to_string(ChronoUtils::GetCurrentTimeMillis() - t1) << "ms" << std::endl;

            itr
                | rpp::operators::first()
                | rpp::operators::subscribe([&](const Document& item) {
                    ASSERT_EQ(item.text(), text);
                });
        }

    }

    TEST_F(DuckDBVectorStoreTest, SearchWithFilter) {
        size_t dim = 128;
        auto db_file_path = INSTINCT_LLM_NS::ensure_random_temp_folder() / "test.db";
        LOG_INFO("db_file_path={}", db_file_path);
        auto embeddings = INSTINCT_LLM_NS::create_pesudo_embedding_model(dim);

        const auto store = CreateDuckDBVectorStore(
            embeddings,
            { .table_name = "test_table_1", .db_file_path = db_file_path, .dimension = dim}
        );

        std::vector<Document> docs;
        for (const int i: std::views::iota (0,100)) {
            Document document;
            document.set_text(std::to_string(i));
            auto* page_no = document.mutable_metadata()->Add();
            page_no->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
            page_no->set_int_value(i);
            DocumentUtils::AddMissingPresetMetadataFields(document);
            docs.push_back(document);
        }
        UpdateResult update_result;
        store->AddDocuments(docs, update_result);

        for(const auto& [text, embedding]: embeddings->get_caches()) {
            // query that excludes top-1 item with sql conditions
            SearchRequest search_request;
            search_request.set_query(text);
            auto* cond1 = search_request.mutable_metadata_filter()->mutable_bool_()->add_mustnot();
            cond1->mutable_term()->set_name(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
            cond1->mutable_term()->mutable_term()->set_int_value(std::stoi(text));
            const auto itr = store->SearchDocuments(search_request);
            itr | rpp::ops::first()
                | rpp::ops::subscribe([&](const Document& document) {
                    ASSERT_TRUE(document.text() != text);
                });
        }

    }






}
