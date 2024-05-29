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
            size_t limit = 10
        ) {
            assert_gt(limit, 0, "limit shoud be positive");
            assert_lt(limit, 1000, "limit shold be less than 1000");

            // ommit vectro field to reduce payload size
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema->fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            select_sql += name_view.empty() ? ""  : ", " + StringUtils::JoinWith(name_view, ", ");
            select_sql += ", array_cosine_similarity(vector, array_value(";

            for (int i = 0; i < query_vector.size(); i++) {
                select_sql += std::to_string(query_vector.at(i)) + "::FLOAT";
                if (i < query_vector.size() - 1) {
                    select_sql += ",";
                }
            }
            select_sql += ")) AS similarity FROM ";
            select_sql += table_name;


            if (metadata_filter.has_bool_() || metadata_filter.has_term()) {
                select_sql += " WHERE ";
                // TODO expand all queries. for now, let's assume it only has one term query at root level
                if (metadata_filter.has_term()) {
                    const auto& field = metadata_filter.term();
                    switch (field.term().kind_case()) {
                        case google::protobuf::Value::kNumberValue: {
                            select_sql += (field.name() + "=" + std::to_string(field.term().number_value()));
                            break;
                        }
                        case google::protobuf::Value::kBoolValue: {
                            select_sql += (field.name() + "=" + std::to_string(field.term().bool_value()));
                            break;
                        }
                        case google::protobuf::Value::kStringValue: {
                            select_sql += (field.name() + "=\"" + field.term().string_value() + "\"");
                            break;
                        }
                        case google::protobuf::Value::kNullValue:
                        case google::protobuf::Value::kStructValue:
                        case google::protobuf::Value::kListValue:
                            // Do nothing
                            break;
                        default:
                            throw InstinctException(
                                "unknown value type in meatdata filter for field named " + field.name());
                    }
                }
            }


            select_sql += " ORDER BY similarity DESC LIMIT ";
            select_sql += std::to_string(limit);
            return select_sql;
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
            LOG_DEBUG("Search started: query={}, top_k={}", request.query(), request.top_k());
            long t1 = ChronoUtils::GetCurrentTimeMillis();
            const auto query_embedding = embeddings_->EmbedQuery(request.query());

            bool has_filter = request.has_metadata_filter() && (request.metadata_filter().has_bool_() || request.metadata_filter().has_term());

            unique_ptr<QueryResult> result;
            if (has_filter) {
                const auto search_sql = details::make_search_sql(
                store_.GetOptions().table_name,
                GetMetadataSchema(),
                query_embedding,
                request.metadata_filter(),
                request.top_k());
                result = store_.GetConnection().Query(search_sql);
            } else {
                // use prepared statement for better performance when no filter is actually given
                vector<duckdb::Value> vector_array;
                for(const float& f: query_embedding) {
                    vector_array.push_back(duckdb::Value::FLOAT(f));
                }
                result = prepared_search_statement_->Execute(duckdb::Value::ARRAY(LogicalType::FLOAT, vector_array), request.top_k());
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
