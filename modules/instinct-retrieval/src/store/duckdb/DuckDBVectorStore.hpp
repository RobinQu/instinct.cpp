//
// Created by RobinQu on 2024/3/6.
//

#ifndef DUCKDBVECTORSTORE_HPP
#define DUCKDBVECTORSTORE_HPP

#include <duckdb.hpp>
#include <retrieval.pb.h>

#include "DuckDBStoreInternal.hpp"
#include "retrieval/IStatefulRetriever.hpp"
#include "store/IVectorStore.hpp"
#include "tools/Assertions.hpp"
#include "tools/StringUtils.hpp"



namespace INSTINCT_RETRIEVAL_NS {
    using namespace duckdb;
    using namespace google::protobuf;


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
            select_sql += ", array_cosine_similarity(vector, ?) AS similarity FROM  ";
            select_sql += table_name;
            select_sql += " ORDER BY similarity DESC LIMIT ?";
            return select_sql;
        }


        static std::string make_search_sql(
            const std::string& table_name,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const std::vector<float>& query_vector,
            const MetadataQuery& metadata_filter,
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
                    auto field = metadata_filter.term().predicate();
                    switch (field.value_case()) {
                        case PrimitiveVariable::kIntValue:
                            select_sql += (field.name() + "=" + std::to_string(field.int_value()));
                            break;
                        case PrimitiveVariable::kLongValue:
                            select_sql += (field.name() + "=" + std::to_string(field.long_value()));
                            break;
                        case PrimitiveVariable::kFloatValue:
                            select_sql += (field.name() + "=" + std::to_string(field.float_value()));
                            break;
                        case PrimitiveVariable::kDoubleValue:
                            select_sql += (field.name() + "=" + std::to_string(field.double_value()));
                            break;
                        case PrimitiveVariable::kBoolValue:
                            select_sql += (field.name() + "=" + std::to_string(field.bool_value()));
                            break;
                        case PrimitiveVariable::kStringValue:
                            select_sql += (field.name() + "=\"" + field.string_value() + "\"");
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



        static void append_row(
                const std::shared_ptr<MetadataSchema>& metadata_schema,
                Appender& appender,
                Document& doc,
                const Embedding& embedding,
                UpdateResult& update_result,
                const bool bypass_unknown_fields
        ) {
            appender.BeginRow();

            // basic fields
            append_row_basic_fields(appender, doc, update_result);

            // column of vector
            vector<Value> vector_value;
            for (const float& f: embedding) {
                vector_value.push_back(Value::FLOAT(f));
            }
            appender.Append(Value::ARRAY(vector_value));

            // metadata fields
            append_row_metadata_fields(metadata_schema, appender, doc, bypass_unknown_fields);

            appender.EndRow();
        }

    }

    class DuckDBVectorStoreInternalAppender final: public DuckDBInternalAppender {
        EmbeddingsPtr embeddings_;
        std::shared_ptr<MetadataSchema> metadata_schema_;
        bool bypass_unknonw_fields_;

    public:
        DuckDBVectorStoreInternalAppender(EmbeddingsPtr embeddings, std::shared_ptr<MetadataSchema> metadata_schema,
            const bool bypass_unknonw_fields)
            : embeddings_(std::move(embeddings)),
              metadata_schema_(std::move(metadata_schema)),
              bypass_unknonw_fields_(bypass_unknonw_fields) {
        }

        void AppendRow(Appender& appender, Document& doc, UpdateResult& update_result) override {
            const auto embeddings = embeddings_->EmbedDocuments({doc.text()});
            details::append_row(metadata_schema_, appender, doc, embeddings[0], update_result, bypass_unknonw_fields_);
        }

        void AppendRows(Appender& appender, std::vector<Document>& records, UpdateResult& update_result) override {
            // std::vector<std::string> ids;
            auto text_view = records | std::views::transform([](auto&& record) -> std::string {
                return record.text();
            });
            auto embeddings = embeddings_->EmbedDocuments({text_view.begin(), text_view.end()});
            assert_equal_size(embeddings, records);
            int affected_row = 0;
            for (int i = 0; i < records.size(); i++) {
                try {
                    details::append_row(metadata_schema_, appender, records[i], embeddings[i], update_result, bypass_unknonw_fields_);
                    affected_row++;
                } catch (const InstinctException& e) {
                    update_result.add_failed_documents()->CopyFrom(records[i]);
                    // TODO with better logging facilities
                    std::cerr << e.what() << std::endl;
                }
            }
            update_result.set_affected_rows(affected_row);
        }
    };

    class DuckDBVectorStore final: public IVectorStore {
        EmbeddingsPtr embeddings_;
        std::unique_ptr<PreparedStatement> prepared_search_statement_;
        std::shared_ptr<DuckDBStoreInternal> internal_;

    public:
        DuckDBVectorStore() = delete;

        explicit DuckDBVectorStore(
            const EmbeddingsPtr& embeddings_model,
            const DuckDBStoreOptions& options,
            const std::shared_ptr<MetadataSchema>& metadata_schema
        ):
            embeddings_(embeddings_model)
        {
            assert_gt(options.dimmension, 0);
            assert_true(embeddings_ != nullptr, "should provide embeddings object pointer");
            assert_true(!!metadata_schema, "should have provide valid metadata schema");
            auto internal_appender = std::make_shared<DuckDBVectorStoreInternalAppender>(embeddings_model, metadata_schema, options.bypass_unknonw_fields);
            internal_ = std::make_shared<DuckDBStoreInternal>(internal_appender, options, metadata_schema);
            prepared_search_statement_ = internal_->GetConnection().Prepare(details::make_prepared_search_sql(options.table_name, metadata_schema));
        }


        EmbeddingsPtr GetEmbeddingModel() override {
            return embeddings_;
        }

        ResultIteratorPtr<Document> SearchDocuments(const SearchRequest& request) override {
            const auto query_embedding = embeddings_->EmbedQuery(request.query());

            bool has_filter = request.has_metadata_filter() && (request.metadata_filter().has_bool_() || request.metadata_filter().has_term());

            unique_ptr<QueryResult> result;
            if (has_filter) {
                const auto search_sql = details::make_search_sql(
                internal_->GetOptions().table_name,
                GetMetadataSchema(),
                query_embedding,
                request.metadata_filter(),
                request.top_k());
                result = internal_->GetConnection().Query(search_sql);
            } else {
                // use prepared statement for better performenace when no filter is actually given
                vector<Value> vector_array;
                for(const float& f: query_embedding) {
                    vector_array.push_back(Value::FLOAT(f));
                }
                result = prepared_search_statement_->Execute(Value::ARRAY(LogicalType::FLOAT, vector_array), request.top_k());
            }
            details::assert_query_ok(result);
            return details::conv_query_result_to_iterator(
                    result.get(),
                    GetMetadataSchema()
            );
        }

        void AddDocuments(const ResultIteratorPtr<Document>& documents_iterator, UpdateResult& update_result) override {
            internal_->AddDocuments(documents_iterator, update_result);
        }

        void AddDocuments(std::vector<Document>& records, UpdateResult& update_result) override {
            internal_->AddDocuments(records, update_result);
        }

        void AddDocument(Document& doc) override {
            internal_->AddDocument(doc);
        }

        size_t DeleteDocuments(const std::vector<std::string>& ids) override {
            return internal_->DeleteDocuments(ids);
        }

        ResultIteratorPtr<Document> MultiGetDocuments(const std::vector<std::string>& ids) override {
            return internal_->MultiGetDocuments(ids);
        }

        [[nodiscard]] std::shared_ptr<MetadataSchema> GetMetadataSchema() const override {
            return internal_->GetMetadataSchema();
        }

        size_t CountDocuments() override {
            return internal_->CountDocuments();
        }
    };


    static VectorStorePtr CreateDuckDBVectorStore(
        const EmbeddingsPtr& embeddings_model,
        const DuckDBStoreOptions& options,
        const std::shared_ptr<MetadataSchema>& metadata_schema = details::EMPTY_METADATA_SCHEMA
    ) {
        return std::make_shared<DuckDBVectorStore>(
            embeddings_model,
            options,
            metadata_schema
        );
    }
}


#endif //DUCKDBVECTORSTORE_HPP
