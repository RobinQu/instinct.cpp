//
// Created by RobinQu on 2024/3/6.
//

#ifndef DUCKDBVECTORSTORE_HPP
#define DUCKDBVECTORSTORE_HPP

#include <duckdb.hpp>
#include <retrieval.pb.h>

#include "BaseDuckDBStore.hpp"
#include "DuckDBDocStore.hpp"
#include "DuckDBDocWithEmbeddingStore.hpp"
#include "store/IVectorStore.hpp"
#include "tools/Assertions.hpp"
#include "tools/StringUtils.hpp"
#include "tools/MetadataSchemaBuilder.hpp"
#include "tools/ChronoUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace duckdb;
    using namespace google::protobuf;
    using namespace INSTINCT_DATA_NS;


    namespace details {

        /**
         * make sql text for prepared statement, ignoring metadata filter
         * @param table_name
         * @param metadata_schema
         * @return
         */
        static std::string make_prepared_search_sql(
            const std::string& table_name,
            const std::shared_ptr<MetadataSchema>& metadata_schema

            ) {
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema->fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            select_sql += name_view.empty() ? ""  : ", " + StringUtils::JoinWith(name_view, ", ");
            select_sql += ", array_cosine_similarity(vector, ?) AS similarity FROM ";
            select_sql += table_name;
            select_sql += " ORDER BY similarity DESC LIMIT ?";
            return select_sql;
        }


        static std::string make_search_sql(
            const std::string& table_name,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const std::vector<float>& query_vector,
            const SearchQuery& metadata_filter,
            const int limit = 10
        ) {
            assert_gt(limit, 0, "limit shoud be positive");
            assert_lt(limit, 1000, "limit shold be less than 1000");

            // omit vector field to reduce payload size
            std::string column_list = "id, text";
            auto name_view = metadata_schema->fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            column_list += name_view.empty() ? ""  : ", " + StringUtils::JoinWith(name_view, ", ");
            column_list += ", array_cosine_similarity(vector, array_value(";

            for (int i = 0; i < query_vector.size(); i++) {
                column_list += std::to_string(query_vector.at(i)) + "::FLOAT";
                if (i < query_vector.size() - 1) {
                    column_list += ",";
                }
            }
            column_list += ")) AS similarity";

            Sorter sorter;
            auto* field_sort = sorter.mutable_field();
            field_sort->set_field_name("similarity");
            field_sort->set_order(DESC);
            return SQLBuilder::ToSelectString(table_name, column_list, metadata_filter, {sorter}, -1, limit);
        }

    }


    /**
     * IVectorStore implementation using brute-force cosine similarly executed by DuckDB instance
     */
    class DuckDBVectorStore final: public virtual IVectorStore {
        DuckDBDocWithEmbeddingStore store_;
        unique_ptr<PreparedStatement> prepared_search_statement_;
        EmbeddingsPtr embeddings_;
    public:
        DuckDBVectorStore() = delete;

        explicit DuckDBVectorStore(
            const DuckDBPtr& db,
            const EmbeddingsPtr& embeddings_model,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const DuckDBStoreOptions& options
        ):  store_(db, metadata_schema, embeddings_model, options),
            embeddings_(embeddings_model)
        {
            assert_gt(options.dimension, 0);
            assert_true(embeddings_ != nullptr, "should provide embeddings object pointer");
            assert_true(embeddings_->GetDimension() == options.dimension, "should have dimension set correctly");
            assert_true(metadata_schema, "should have provide valid metadata schema");

            auto search_sql = details::make_prepared_search_sql(options.table_name, metadata_schema);
            LOG_DEBUG("prepare search sql: {}", search_sql);
            prepared_search_statement_ =  store_.GetConnection().Prepare(search_sql);
            assert_prepared_ok(prepared_search_statement_, "Failed to prepare search statement");
        }


        EmbeddingsPtr GetEmbeddingModel() override {
            return embeddings_;
        }

        AsyncIterator<Document> SearchDocuments(const SearchRequest& request) override {
            const int limit = request.top_k() > 0 ? std::min(request.top_k(), 10000) : 10;
            LOG_DEBUG("Search started: request.query={}, request.top_k={}, normalized_limit={}", request.query(), request.top_k(), limit);
            long t1 = ChronoUtils::GetCurrentTimeMillis();
            const auto query_embedding = embeddings_->EmbedQuery(request.query());
            const bool has_filter = request.has_metadata_filter() && (request.metadata_filter().has_bool_() || request.metadata_filter().has_term());
            unique_ptr<QueryResult> result;
            // limit should be in range of [1,10000]

            if (has_filter) {
                const auto search_sql = details::make_search_sql(
                store_.GetOptions().table_name,
                GetMetadataSchema(),
                query_embedding,
                request.metadata_filter(),
                limit);
                LOG_DEBUG("search_sql: {}", search_sql);
                result = store_.GetConnection().Query(search_sql);
            } else {
                // use prepared statement for better performance when no filter is actually given
                vector<duckdb::Value> vector_array;
                for(const float& f: query_embedding) {
                    vector_array.push_back(duckdb::Value::FLOAT(f));
                }
                result = prepared_search_statement_->Execute(duckdb::Value::ARRAY(LogicalType::FLOAT, vector_array), limit);
            }
            assert_query_ok(result);
            return details::conv_query_result_to_iterator(
                    std::move(result),
                    GetMetadataSchema()
            ) | rpp::operators::tap({}, {}, [t1]() {
                LOG_INFO("Search done, rt={}ms", ChronoUtils::GetCurrentTimeMillis()-t1);
            });
        }

        void AddDocuments(const AsyncIterator<Document>& documents_iterator, UpdateResult& update_result) override {
            store_.AddDocuments(documents_iterator, update_result);
        }

        void AddDocuments(const std::vector<Document>& records, UpdateResult& update_result) override {
            store_.AddDocuments(records, update_result);
        }

        void AddDocument(Document& doc) override {
            store_.AddDocument(doc);
        }

        void DeleteDocuments(const std::vector<std::string>& ids, UpdateResult& update_result) override {
            store_.DeleteDocuments(ids, update_result);
        }

        AsyncIterator<Document> MultiGetDocuments(const std::vector<std::string>& ids) override {
            return store_.MultiGetDocuments(ids);
        }

        [[nodiscard]] std::shared_ptr<MetadataSchema> GetMetadataSchema() const override {
            return store_.GetMetadataSchema();
        }

        size_t CountDocuments() override {
            return store_.CountDocuments();
        }

        void DeleteDocuments(const SearchQuery &filter, UpdateResult &update_result) override {
            store_.DeleteDocuments(filter, update_result);
        }

        bool Destroy() override {
            return store_.Destroy();
        }
    };

    /**
     *
     * @param db A shared DuckDb instance
     * @param embeddings_model
     * @param options
     * @param metadata_schema
     * @return
     */
    static VectorStorePtr CreateDuckDBVectorStore(
            const DuckDBPtr& db,
            const EmbeddingsPtr& embeddings_model,
            const DuckDBStoreOptions& options,
            MetadataSchemaPtr metadata_schema = nullptr
        ) {
        if (!metadata_schema) {
            metadata_schema = CreateVectorStorePresetMetadataSchema();
        }
        return std::make_shared<DuckDBVectorStore>(
            db,
            embeddings_model,
            metadata_schema,
            options
        );

    }

    /**
     * Create VectorStore using duckdb implementation
     * @param embeddings_model
     * @param options
     * @param metadata_schema Schema for vector table. Typically empty schema will do.
     * @return
     */
    static VectorStorePtr CreateDuckDBVectorStore(
        const EmbeddingsPtr& embeddings_model,
        const DuckDBStoreOptions& options,
        const MetadataSchemaPtr& metadata_schema = nullptr
    ) {
        return CreateDuckDBVectorStore(
            options.in_memory ? std::make_shared<DuckDB>(nullptr) : std::make_shared<DuckDB>(options.db_file_path),
            embeddings_model,
            options,
            metadata_schema
            );
    }

}


#endif //DUCKDBVECTORSTORE_HPP
