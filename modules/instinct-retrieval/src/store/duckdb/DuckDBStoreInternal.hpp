//
// Created by RobinQu on 2024/3/13.
//

#ifndef BASEDUCKDBSTORE_HPP
#define BASEDUCKDBSTORE_HPP

#include "RetrievalGlobals.hpp"
#include <duckdb.hpp>
#include "tools/Assertions.hpp"

#include "LLMGlobals.hpp"
#include "store/IDocStore.hpp"
#include "tools/ResultIterator.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace duckdb;
    using namespace google::protobuf;
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    struct DuckDBStoreOptions {
        std::string table_name;
        std::string db_file_path;
        size_t dimmension = 0;
        bool bypass_unknonw_fields = true;
    };



    namespace details {
        /**
         * Empty value for MetadataSchema
         */
        static auto EMPTY_METADATA_SCHEMA = std::make_shared<MetadataSchema>();


        static std::string make_preapred_mget_sql (
            const std::string& table_name,
            const std::shared_ptr<MetadataSchema>& metadata_schema
            ) {
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema->fields() | std::views::transform(
                                             [](const MetadataFieldSchema& field)-> std::string {
                                                 return field.name();
                                             });
            select_sql += name_view.empty() ? ""  : ", " + StringUtils::JoinWith(name_view, ", ");
            select_sql += " FROM ";
            select_sql += table_name;
            select_sql += " WHERE id in ?";
            return select_sql;
        }

        static std::string make_prepared_count_sql (const std::string& table_name) {
            return fmt::format("SELECT count(*) from {}", table_name);
        }

        static std::string make_create_table_sql(
            const std::string& table_name,
            const size_t dimmension,
            const std::shared_ptr<MetadataSchema>& metadata_schema
        ) {
            auto create_table_sql = "CREATE OR REPLACE TABLE " + table_name + "(";
            std::vector<std::string> parts;
            parts.emplace_back("id VARCHAR PRIMARY KEY");
            parts.emplace_back("text VARCHAR NOT NULL");
            if (dimmension > 0) {
                parts.emplace_back("vector FLOAT[" + std::to_string(dimmension) + "] NOT NULL");
            }

            for (const auto& field: metadata_schema->fields()) {
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


        static ResultIteratorPtr<Document> conv_query_result_to_iterator(
            QueryResult* result,
            const std::shared_ptr<MetadataSchema>& metadata_schema_) {
            std::vector<Document> docs;
            for (const auto& row: *result) {
                Document document;
                document.set_id(row.GetValue<std::string>(0));
                document.set_text(row.GetValue<std::string>(1));
                // for (const Value vector_value = row.GetValue<Value>(2);
                //     const auto& v: ArrayValue::GetChildren(vector_value)) {
                //     document.add_vector(v.GetValue<float>());
                // }
                for (int i = 0; i < metadata_schema_->fields_size(); i++) {
                    int column_idx = i + 2;
                    auto& field_schema = metadata_schema_->fields(i);
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
            return create_result_itr_from_range(docs);

        }


        static void append_row_basic_fields(
            Appender& appender,
            Document& doc,
            UpdateResult& update_result
            ) {
            // column of id
            std::string new_id = u8_utils::uuid_v8();
            update_result.add_returned_ids(new_id);
            appender.Append<>(new_id.c_str());
            doc.set_id(new_id);

            // column of text
            // TODO escape text chars in case of sql injection
            appender.Append<>(doc.text().c_str());
        }

        static void append_row_metadata_fields(
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            Appender& appender,
            Document& doc,
            bool bypass_unknown_fields
        ) {
            if(metadata_schema == EMPTY_METADATA_SCHEMA) {
                return;
            }

            // handle rows defined in metadata_schema
            auto name_view = metadata_schema->fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            std::unordered_set<std::string> known_field_names{name_view.begin(), name_view.end()};

            // columns of metadata
            auto* schema = Document::GetDescriptor();
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

                if (!bypass_unknown_fields) {
                    assert_true((known_field_names.contains(name)),
                                "Metadata cannot contain field not defined by schema. Problematic field is " + name);
                }
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
        }
    }


    class DuckDBInternalAppender {
    public:
        virtual ~DuckDBInternalAppender() = default;

        virtual void AppendRow(Appender& appender, Document& doc, UpdateResult& update_result) = 0;

        virtual void AppendRows(Appender& appender, std::vector<Document>& records, UpdateResult& update_result) = 0;
    };
    using DuckDBInternalAppenderPt = std::shared_ptr<DuckDBInternalAppender>;

    class DuckDBStoreInternal: public virtual IDocStore {
        DuckDBStoreOptions options_;
        DuckDB db_;
        std::shared_ptr<MetadataSchema> metadata_schema_;
        Connection connection_;
        std::unique_ptr<PreparedStatement> prepared_mget_statement_;
        std::unique_ptr<PreparedStatement> prepared_count_all_statement_;
        DuckDBInternalAppenderPt internal_appender_;
    public:
        explicit DuckDBStoreInternal(
            const DuckDBInternalAppenderPt& internal_appender,
            const DuckDBStoreOptions& options,
            const std::shared_ptr<MetadataSchema>& metadata_schema
            ):
            options_(options),
                    db_(options.db_file_path),
                    metadata_schema_(metadata_schema),
                    connection_(db_),
                    internal_appender_(internal_appender)
                     {
            assert_true(!!metadata_schema, "should provide shcema");
            // assert_positive(options_.dimmension, "dimension should be positive");
            assert_lt(options_.dimmension, 10000, "dimension should be less than 10000");
            assert_true(!StringUtils::IsBlankString(options_.table_name), "table_name cannot be blank");


            const auto sql = details::make_create_table_sql(options_.table_name, options_.dimmension, metadata_schema_);
            const auto create_table_result = connection_.Query(sql);
            details::assert_query_ok(create_table_result);
            prepared_mget_statement_ = connection_.Prepare(details::make_preapred_mget_sql(options_.table_name, metadata_schema_));
            prepared_count_all_statement_ = connection_.Prepare(details::make_prepared_count_sql(options_.table_name));
        }


        [[nodiscard]] const DuckDBStoreOptions& GetOptions() const {
            return options_;
        }

        [[nodiscard]] std::shared_ptr<MetadataSchema> GetMetadataSchema() const override {
            return metadata_schema_;
        }

        [[nodiscard]] Connection& GetConnection() {
            return connection_;
        }

        ResultIteratorPtr<Document> MultiGetDocuments(const std::vector<std::string>& ids) override {
            vector<Value> id_vector;
            for(const auto& id: ids) {
                id_vector.push_back(id);
            }
            const auto result = prepared_mget_statement_->Execute(id_vector);
            details::assert_query_ok(result);
            return details::conv_query_result_to_iterator(result.get(), metadata_schema_);
        }

        size_t CountDocuments() override {
            const auto result = prepared_count_all_statement_->Execute();
            details::assert_query_ok(result);
            for (const auto& row: *result) {
                return  row.GetValue<uint32_t>(0);
            }
            throw InstinctException("Empty count result");
        }

        void AddDocuments(const ResultIteratorPtr<Document>& documents_iterator, UpdateResult& update_result) override {
            static size_t batch_size = 10;
            std::vector<std::string> ids;
            std::vector<Document> batch(batch_size);
            while (documents_iterator->HasNext()) {
                const auto doc = documents_iterator->Next();
                batch.push_back(doc);
                if (batch.size() == batch_size) {
                    AddDocuments(batch, update_result);
                    batch.clear();
                }
            }
            if (!batch.empty()) {
                AddDocuments(batch, update_result);
            }
        }

        void AddDocuments(std::vector<Document>& records, UpdateResult& update_result) override {
            Appender appender(connection_, options_.table_name);
            // // std::vector<std::string> ids;
            // auto text_view = records | std::views::transform([](auto&& record) -> std::string {
            //     return record.text();
            // });
            // auto embeddings = embeddings_->EmbedDocuments({text_view.begin(), text_view.end()});
            // assert_equal_size(embeddings, records);
            // int affected_row = 0;
            // for (int i = 0; i < records.size(); i++) {
            //     try {
            //         // details::append_row(metadata_schema_, appender, records[i], embeddings[i], update_result, options_.bypass_unknonw_fields);
            //         affected_row++;
            //     } catch (const InstinctException& e) {
            //         update_result.add_failed_documents()->CopyFrom(records[i]);
            //         // TODO with better logging facilities
            //         std::cerr << e.what() << std::endl;
            //     }
            // }
            // update_result.set_affected_rows(affected_row);

            internal_appender_->AppendRows(appender, records, update_result);
            appender.Close();
        }

        void AddDocument(Document& doc) override {
            UpdateResult update_result;
            Appender appender(connection_, options_.table_name);
            // const auto embeddings = embeddings_->EmbedDocuments({doc.text()});
            // details::append_row(metadata_schema_, appender, doc, embeddings[0], update_result, options_.bypass_unknonw_fields);
            internal_appender_->AppendRow(appender, doc, update_result);
            appender.Close();
        }

        size_t DeleteDocuments(const std::vector<std::string>& ids) override {
            const auto sql = details::make_delete_sql(options_.table_name, ids);
            const auto result = connection_.Query(sql);
            details::assert_query_ok(result);
            return result->GetValue<int32_t>(0, 0);
        }

        using DuckDBStoreInternalPtr = std::shared_ptr<DuckDBStoreInternal>;



    };

}

#endif //BASEDUCKDBSTORE_HPP
