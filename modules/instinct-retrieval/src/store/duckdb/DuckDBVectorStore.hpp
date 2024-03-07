//
// Created by RobinQu on 2024/3/6.
//

#ifndef DUCKDBVECTORSTORE_HPP
#define DUCKDBVECTORSTORE_HPP

#include <duckdb.hpp>
#include "store/VectorStore.hpp"
#include "tools/Assertions.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace duckdb;
    using namespace google::protobuf;

    struct DuckDbVectoreStoreOptions {
        std::string table_name;
        std::string db_file_path;
        size_t dimmension = 128;
        MetadataSchema metadata_schema;
        EmbeddingsPtr embeddings;
    };

    namespace details {
        /**
         * make sql text for prepared statement, ignoring metadata filter
         * @param table_name
         * @param metadata_schema
         * @return
         */
        static std::string make_prepared_search_sql(
            const std::string& table_name,
            const MetadataSchema& metadata_schema

            ) {
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema.fields() | std::views::transform(
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
            const MetadataSchema& metadata_schema,
            const std::vector<float>& query_vector,
            const MetadataQuery& metadata_filter,
            size_t limit = 10
        ) {
            assert_gt(limit, 0, "limit shoud be positive");
            assert_lt(limit, 1000, "limit shold be less than 1000");

            // ommit vectro field to reduce payload size
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema.fields() | std::views::transform(
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
                        case MetadataField::kIntValue:
                            select_sql += (field.name() + "=" + std::to_string(field.int_value()));
                            break;
                        case MetadataField::kLongValue:
                            select_sql += (field.name() + "=" + std::to_string(field.long_value()));
                            break;
                        case MetadataField::kFloatValue:
                            select_sql += (field.name() + "=" + std::to_string(field.float_value()));
                            break;
                        case MetadataField::kDoubleValue:
                            select_sql += (field.name() + "=" + std::to_string(field.double_value()));
                            break;
                        case MetadataField::kBoolValue:
                            select_sql += (field.name() + "=" + std::to_string(field.bool_value()));
                            break;
                        case MetadataField::kStringValue:
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

        static std::string make_create_table_sql(
            const std::string& table_name,
            const size_t dimmension,
            const MetadataSchema& metadata_schema
        ) {
            auto create_table_sql = "CREATE OR REPLACE TABLE " + table_name + "(";
            std::vector<std::string> parts;
            parts.emplace_back("id VARCHAR PRIMARY KEY");
            parts.emplace_back("text VARCHAR NOT NULL");
            parts.emplace_back("vector FLOAT[" + std::to_string(dimmension) + "] NOT NULL");
            for (const auto& field: metadata_schema.fields()) {
                auto mfd = field.name() + " ";
                switch (field.type()) {
                    case INT32:
                        mfd += "INTEGER";
                        break;
                    case INT64:
                        mfd += "BIGINT";
                        break;
                    case FLOAT:
                        mfd += "FLOAT";
                        break;
                    case DOUBLE:
                        mfd += "DOUBLE";
                        break;
                    case VARCHAR:
                        mfd += "VARCHAR";
                        break;
                    case BOOL:
                        mfd += "BOOL";
                        break;
                    default:
                        throw InstinctException("unknown field type :" + std::string(field.name()));
                }
                parts.push_back(mfd);
            }
            create_table_sql += StringUtils::JoinWith(parts, ", ");
            create_table_sql += ");";
            return create_table_sql;
        }

        static std::string make_delete_sql(const std::string& table_name, const std::vector<std::string>& ids) {
            assert_non_empty_range(ids, "ids should not be empty");
            std::string delete_sql = "DELETE FROM " + table_name + " WHERE id IN (";
            delete_sql += StringUtils::JoinWith(ids, ", ");
            delete_sql += ");";
            return delete_sql;
        }


        static void assert_query_ok(const unique_ptr<MaterializedQueryResult>& result) {
            if (const auto error = result->GetErrorObject(); error.HasError()) {
                throw InstinctException(result->GetError());
            }
        }

        static void assert_query_ok(const unique_ptr<QueryResult>& result) {
            if (const auto error = result->GetErrorObject(); error.HasError()) {
                throw InstinctException(result->GetError());
            }
        }


        static void append_row(const MetadataSchema& metadata_schema, Appender& appender, const Document& doc,
                               const Embedding& embedding,
                               std::vector<std::string>& ids_out) {
            auto* schema = Document::GetDescriptor();
            appender.BeginRow();
            // column of id
            ids_out.push_back(u8_utils::uuid_v8());
            appender.Append<>(ids_out.back().c_str());

            // column of text
            // TODO escape text chars in case of sql injection
            appender.Append<>(doc.text().c_str());

            // column vector
            vector<Value> vector_value;
            for (const float& f: embedding) {
                vector_value.push_back(Value::FLOAT(f));
            }
            appender.Append(Value::ARRAY(vector_value));

            auto name_view = metadata_schema.fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            std::unordered_set<std::string> known_field_names{name_view.begin(), name_view.end()};

            // columns of metadata
            auto* reflection = Document::GetReflection();
            const auto* metadata_field = schema->FindFieldByName("metadata");
            int metadata_size = reflection->FieldSize(doc, metadata_field);
            for (int i = 0; i < metadata_size; i++) {
                const auto& metadata_field_message = reflection->GetRepeatedMessage(
                    doc,
                    metadata_field,
                    i);
                auto metadata_field_descriptor = metadata_field_message.GetDescriptor();
                auto* field_value_reflection = metadata_field_message.GetReflection();
                auto name_field_desccriptor = metadata_field_descriptor->FindFieldByName("name");
                auto name = field_value_reflection->GetString(metadata_field_message, name_field_desccriptor);

                assert_true((known_field_names.contains(name)),
                            "metadata cannot contains field not defined by schema. Field in question is " + name);
                known_field_names.erase(name);

                auto* value_descriptor = metadata_field_descriptor->FindOneofByName("value");
                const auto* field_descriptor = field_value_reflection->GetOneofFieldDescriptor(doc, value_descriptor);

                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_INT32:
                        appender.Append<int32_t>(
                            field_value_reflection->GetInt32(metadata_field_message, field_descriptor));
                        break;
                    case FieldDescriptor::CPPTYPE_INT64:
                        appender.Append<int64_t>(
                            field_value_reflection->GetInt64(metadata_field_message, field_descriptor));
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        appender.Append<
                            bool>(field_value_reflection->GetBool(metadata_field_message, field_descriptor));
                        break;
                    case FieldDescriptor::CPPTYPE_STRING:
                        appender.Append<>(
                            field_value_reflection->GetString(metadata_field_message, field_descriptor).c_str());
                        break;
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        appender.Append<float>(
                            field_value_reflection->GetFloat(metadata_field_message, field_descriptor));
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                        appender.Append<double>(
                            field_value_reflection->GetDouble(metadata_field_message, field_descriptor));
                        break;
                    default:
                        throw InstinctException("unknown field type for appending: " + field_descriptor->name());
                }
            }

            assert_true(known_field_names.empty(), "Some metadata fields not set: " + StringUtils::JoinWith(known_field_names, ","));
            appender.EndRow();
        }
    }


    class DuckDBVectorStore : public VectorStore {
        std::string table_name_;
        DuckDB db_;
        size_t dimmension_;
        // non-owning pointer to message descriptor
        MetadataSchema metadata_schema_;
        Connection connection_;
        EmbeddingsPtr embeddings_;
        std::unique_ptr<PreparedStatement> prepared_search_statement_;

    public:
        DuckDBVectorStore() = delete;

        explicit DuckDBVectorStore(const DuckDbVectoreStoreOptions& options): table_name_(options.table_name),
                                                                              db_(options.db_file_path),
                                                                              metadata_schema_(options.metadata_schema),
                                                                              dimmension_(options.dimmension),
                                                                              connection_(db_),
                                                                              embeddings_(options.embeddings) {
            assert_positive(dimmension_, "dimension should be positive");
            assert_lt(dimmension_, 10000, "dimension should be less than 10000");
            assert_true(embeddings_ != nullptr, "should provide embeddings object pointer");
            assert_non_empty_range(table_name_, "table_name cannot be empty");
            InitializeDB_();
        }

        std::vector<std::string> AddDocuments(ResultIterator<Document>* documents_iterator) override {
            static size_t batch_size = 10;
            std::vector<std::string> ids;
            std::vector<Document> batch(batch_size);
            while (documents_iterator->HasNext()) {
                const auto doc = documents_iterator->Next();
                batch.push_back(doc);
                if (batch.size() == batch_size) {
                    AddDocuments(batch, ids);
                    batch.clear();
                }
            }
            if (!batch.empty()) {
                AddDocuments(batch, ids);
            }
            return ids;
        }

        void AddDocuments(std::vector<Document>& records, std::vector<std::string>& id_result) override {
            Appender appender(connection_, table_name_);
            // std::vector<std::string> ids;
            auto text_view = records | std::views::transform([](auto&& record) -> std::string {
                return record.text();
            });
            auto embeddings = embeddings_->EmbedDocuments({text_view.begin(), text_view.end()});
            assert_equal_size(embeddings, records);
            for (int i = 0; i < records.size(); i++) {
                try {
                    details::append_row(metadata_schema_, appender, records[i], embeddings[i], id_result);
                } catch (const InstinctException& e) {
                    // TODO with better logging facilities
                    std::cerr << e.what() << std::endl;
                }
            }
            appender.Close();
            // return ids;
        }

        size_t DeleteDocuments(const std::vector<std::string>& ids) override {
            const auto sql = details::make_delete_sql(table_name_, ids);
            const auto result = connection_.Query(sql);
            details::assert_query_ok(result);
            return result->GetValue<int32_t>(0, 0);
        }

        ResultIterator<Document>* SearchDocuments(const SearchRequest& request) override {
            const auto query_embedding = embeddings_->EmbedQuery(request.query());

            bool has_filter = request.has_metadata_filter() && (request.metadata_filter().has_bool_() || request.metadata_filter().has_term());

            unique_ptr<QueryResult> result;
            if (has_filter) {
                const auto search_sql = details::make_search_sql(
                table_name_,
                metadata_schema_,
                query_embedding,
                request.metadata_filter(),
                request.top_k());
                result = connection_.Query(search_sql);
            } else {
                // use prepared statement for better performenace when no filter is actually given
                vector<Value> vector_array;
                for(const float& f: query_embedding) {
                    vector_array.push_back(Value::FLOAT(f));
                }
                result = prepared_search_statement_->Execute(Value::ARRAY(LogicalType::FLOAT, vector_array), request.top_k());
            }
            details::assert_query_ok(result);
            std::vector<Document> docs;
            for (const auto& row: *result) {
                Document document;
                document.set_id(row.GetValue<std::string>(0));
                document.set_text(row.GetValue<std::string>(1));
                // for (const Value vector_value = row.GetValue<Value>(2);
                //     const auto& v: ArrayValue::GetChildren(vector_value)) {
                //     document.add_vector(v.GetValue<float>());
                // }
                for (int i = 0; i < metadata_schema_.fields_size(); i++) {
                    int column_idx = i + 2;
                    auto field_schema = metadata_schema_.fields(i);
                    auto* metadata_field = document.add_metadata();
                    metadata_field->set_name(field_schema.name());
                    switch (field_schema.type()) {
                        case INT32:
                            metadata_field->set_int_value(row.GetValue<int32_t>(column_idx));
                            break;
                        case INT64:
                            metadata_field->set_long_value(row.GetValue<int64_t>(column_idx));
                            break;
                        case FLOAT:
                            metadata_field->set_float_value(row.GetValue<float>(column_idx));
                            break;
                        case DOUBLE:
                            metadata_field->set_double_value(row.GetValue<double>(column_idx));
                            break;
                        case VARCHAR:
                            metadata_field->set_string_value(row.GetValue<std::string>(column_idx));
                            break;
                        case BOOL:
                            metadata_field->set_bool_value(row.GetValue<bool>(column_idx));
                            break;
                        default:
                            throw InstinctException("unknown field type for field named " + field_schema.name());
                    }
                }

                docs.push_back(document);
            }
            return create_from_range(docs);
        }

    private:
        void InitializeDB_() {
            const auto sql = details::make_create_table_sql(table_name_, dimmension_, metadata_schema_);
            const auto create_table_result = connection_.Query(sql);
            details::assert_query_ok(create_table_result);
            prepared_search_statement_ = connection_.Prepare(details::make_prepared_search_sql(table_name_, metadata_schema_));
        }
    };
}


#endif //DUCKDBVECTORSTORE_HPP
