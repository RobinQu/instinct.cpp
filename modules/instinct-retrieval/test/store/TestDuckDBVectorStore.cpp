//
// Created by RobinQu on 2024/3/6.
//
#include <gtest/gtest.h>
#include <random>
#include <ranges>
#include "RetrievalGlobals.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    static Embedding make_zero_vector(const size_t dim=128) {
        Embedding embedding (dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(0);
        }
        return embedding;
    }

    static Embedding make_random_vector(const size_t dim=128) {
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<float> dis(0, 1.0);

        Embedding embedding;
        embedding.reserve(dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(dis(gen));
        }
        return embedding;
    }

    class PesuodoEmbeddings final: public IEmbeddingModel {
        std::unordered_map<std::string, Embedding> caches_ = {};
    public:
        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for(const auto& text: texts) {
                if (!caches_.contains(text)) {
                    caches_.emplace(text, make_random_vector());
                }
                result.push_back(caches_.at(text));
            }
            return result;
        }

        auto& get_caches()  const {
            return caches_;
        }


        Embedding EmbedQuery(const std::string& text) override {
            if (!caches_.contains(text)) {
                caches_.emplace(text, make_random_vector());
            }
            return caches_.at(text);
        }
    };

    class DuckDBVectorStoreTest: public testing::Test {
    protected:
        void SetUp() override {
            auto* name_field = s1.add_fields();
            name_field->set_name("name");
            name_field->set_type(VARCHAR);
            auto* address_field = s1.add_fields();
            address_field->set_name("address");
            address_field->set_type(VARCHAR);
            auto* age_field = s1.add_fields();
            age_field->set_name("age");
            age_field->set_type(INT32);
        }

        MetadataSchema s1;
    };

    TEST_F(DuckDBVectorStoreTest, make_create_table_sql) {
        const auto sql = details::make_create_table_sql("test_tb1", 128, s1);
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "CREATE OR REPLACE TABLE test_tb1(id VARCHAR PRIMARY KEY, text VARCHAR NOT NULL, vector FLOAT[128] NOT NULL, name VARCHAR, address VARCHAR, age INTEGER);");
    }

    TEST_F(DuckDBVectorStoreTest, make_search_sql) {
        MetadataQuery mq;
        auto predicate = mq.mutable_term()
        ->mutable_predicate();
        predicate->set_name("address");
        predicate->set_string_value("shanghai");
        auto sql = details::make_search_sql("tb1", s1, make_zero_vector(), mq);
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "SELECT id, text, name, address, age, array_cosine_similarity(vector, array_value(0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT)) AS similarity FROM tb1 WHERE address=\"shanghai\" ORDER BY similarity DESC LIMIT 10");

        sql = details::make_search_sql("tb1", s1, make_zero_vector(), MetadataQuery {});
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "SELECT id, text, name, address, age, array_cosine_similarity(vector, array_value(0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT,0.000000::FLOAT)) AS similarity FROM tb1 ORDER BY similarity DESC LIMIT 10");

    }

    TEST_F(DuckDBVectorStoreTest, make_delete_sql) {
        auto sql = details::make_delete_sql("tb1", std::vector<std::string>{"1","2","3","4"});
        std::cout << sql << std::endl;
        ASSERT_EQ(sql, "DELETE FROM tb1 WHERE id IN (1, 2, 3, 4);");
    }

    TEST_F(DuckDBVectorStoreTest, TestSimpleRecall) {
        auto db_file_path = std::filesystem::temp_directory_path() / "insintct-retrieval";
        std::filesystem::create_directories(db_file_path);
        db_file_path /= (u8_utils::uuid_v8() + ".db");
        auto embeddings = std::make_shared<PesuodoEmbeddings>();
        DuckDBVectorStore store({.db_file_path = db_file_path, .dimmension = 128, .embeddings = embeddings, .table_name = "test_table_1"});

        std::vector<Document> docs;
        for (int i: std::views::iota (0,1000)) {
            Document document;
            document.set_text(std::to_string(i));
            docs.push_back(document);
        }
        UpdateResult update_result;
        store.AddDocuments(docs, update_result);

        for(const auto& [text, embedding]: embeddings->get_caches()) {
            auto t1 = ChronoUtils::GetCurrentTimeMillis();
            SearchRequest search_request;
            search_request.set_query(text);
            search_request.set_top_k(5);
            auto itr = store.SearchDocuments(search_request);
            std::cout << "search done in " << std::to_string(ChronoUtils::GetCurrentTimeMillis() - t1) << "ms" << std::endl;
            ASSERT_TRUE(itr->HasNext());
            auto doc = itr->Next();
            ASSERT_EQ(doc.text(), text);
        }
    }






}
