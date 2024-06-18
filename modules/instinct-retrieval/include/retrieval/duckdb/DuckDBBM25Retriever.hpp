//
// Created by RobinQu on 2024/6/17.
//

#ifndef DUCKDBFTSRETRIEVER_HPP
#define DUCKDBFTSRETRIEVER_HPP
#include <fmt/format.h>
#include <fmt/args.h>

#include "RetrievalGlobals.hpp"
#include "../BaseRetriever.hpp"
#include "store/duckdb/DuckDBDocStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    struct DuckDBBM25RetrieverOptions {
        std::string id_column_name = "id";
        std::string text_column_name = "text";
        bool auto_build = false;
        std::string stemmer = "english";
        std::string stopwords = "english";
        std::string ignore = R"((\\.|[^a-z])+)";
        bool strip_accents = true;
        bool lower = true;
    };

    /**
    * A readonly retriever class that is built upon DuckDb's fulltext search ability.
    * Documents can be added by calling `Ingest` method or directly added to underlying `doc_store` and rebuilding index manually.
    * https://duckdb.org/docs/extensions/full_text_search.html
    */
    class DuckDBBM25Retriever final: public BaseStatefulRetriever {
        std::shared_ptr<DuckDBDocStore> doc_store_;
        DuckDBBM25RetrieverOptions options_;

    public:
        DuckDBBM25Retriever(const DocStorePtr& doc_store, DuckDBBM25RetrieverOptions options)
            : doc_store_(nullptr),
              options_(std::move(options)) {
            const auto ptr = std::dynamic_pointer_cast<DuckDBDocStore>(doc_store);
            assert_true(ptr, "should assign a pointer to DuckDBDocStore");
            doc_store_ = ptr;
            const auto init_result = doc_store_->GetConnection().Query(R"(INSTALL fts;
LOAD fts;)");
            assert_query_ok(init_result);
            if (options_.auto_build) {
                BuildIndex();
            }
        }

        void BuildIndex(bool overwrite = true) const {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(fmt::arg("table_name", doc_store_->GetOptions().table_name));
            store.push_back(fmt::arg("id_column_name", options_.id_column_name));
            store.push_back(fmt::arg("text_column_name", options_.text_column_name));
            store.push_back(fmt::arg("stemmer", options_.stemmer));
            store.push_back(fmt::arg("stopwords", options_.stopwords));
            store.push_back(fmt::arg("strip_accents", options_.strip_accents));
            store.push_back(fmt::arg("lower", options_.lower));
            store.push_back(fmt::arg("overwrite", overwrite));
            const auto sql = fmt::vformat(R"(PRAGMA create_fts_index({table_name}, {id_column_name}, {text_column_name}, stemmer='{stemmer}', stopwords='{stopwords}', strip_accents={strip_accents}, lower={lower}, overwrite={overwrite});)", store);
            trace_span span {"BuildIndex with sql " + sql};
            const auto result = doc_store_->GetConnection().Query(sql);
            assert_query_ok(result);
        }

        void DropIndex() const {
            const auto sql = fmt::format("PRAGMA drop_fts_index({})", doc_store_->GetOptions().table_name);
            trace_span span {"DropIndex with sql " + sql};
            const auto result = doc_store_->GetConnection().Query(sql);
            assert_query_ok(result);
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest &search_request) const override {
            static int BUFFER_SIZE = 20;
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(fmt::arg("table_name", doc_store_->GetOptions().table_name));
            store.push_back(fmt::arg("id_column_name", options_.id_column_name));
            store.push_back(fmt::arg("text_column_name", options_.text_column_name));
            store.push_back(fmt::arg("query", search_request.query()));
            const auto column_list = fmt::vformat("fts_main_{table_name}.match_bm25({id_column_name}, '{query}', fields := '{text_column_name}') AS score, {id_column_name}", store);
            Sorter sorter;
            auto* field_sort = sorter.mutable_field();
            field_sort->set_field_name("score");
            field_sort->set_order(DESC);
            const auto sql = SQLBuilder::ToSelectString(doc_store_->GetOptions().table_name, column_list, search_request.metadata_filter(), {sorter}, 0, search_request.top_k());
            LOG_DEBUG("sql={}", sql);
            const auto t1 = ChronoUtils::GetCurrentTimeMillis();
            return rpp::source::create<std::string>([&, sql](const auto& ob) {
                const auto result = doc_store_->GetConnection().Query(sql);
                assert_query_ok(result);
                for(const auto& row: *result) {
                    ob.on_next(row.GetValue<std::string>(1));
                }
                ob.on_completed();
            })
                | rpp::ops::buffer(BUFFER_SIZE)
                | rpp::ops::flat_map([&](const std::vector<std::string>& doc_ids) {
                    return doc_store_->MultiGetDocuments(doc_ids);
                })
                | rpp::ops::tap({}, {}, [t1] {
                    LOG_INFO("DuckDBBM25Retriever::Retrieve done, rt={}ms", ChronoUtils::GetCurrentTimeMillis()-t1);
                });
        }

        void Remove(const SearchQuery &metadata_query) override {
            UpdateResult update_result;
            doc_store_->DeleteDocuments(metadata_query, update_result);
            assert_true(update_result.failed_documents_size()==0);
        }

        void Ingest(const AsyncIterator<Document> &input) override {
            input
                | rpp::ops::as_blocking()
                | rpp::ops::buffer(20)
                | rpp::ops::subscribe([&](const std::vector<Document>& docs) {
                    UpdateResult update_result;
                    std::vector<Document> copied = docs;
                    doc_store_->AddDocuments(copied, update_result);
                    assert_true(update_result.failed_documents_size()==0);
                });
            LOG_DEBUG("doc_store_->CountDocuments()={}", doc_store_->CountDocuments());
            BuildIndex(true);
        }

        DocStorePtr GetDocStore() override {
            return doc_store_;
        }

    };

    static StatefulRetrieverPtr CreateDuckDBBM25Retriever(const DocStorePtr& doc_store, const DuckDBBM25RetrieverOptions& options = {}) {
        return std::make_shared<DuckDBBM25Retriever>(doc_store, options);
    }
}


#endif //DUCKDBFTSRETRIEVER_HPP
