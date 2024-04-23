//
// Created by RobinQu on 2024/3/13.
//

#ifndef BASEDUCKDBSTORE_HPP
#define BASEDUCKDBSTORE_HPP

#include <duckdb.hpp>
#include "tools/Assertions.hpp"

#include "RetrievalGlobals.hpp"
#include "store/IDocStore.hpp"
#include "tools/StringUtils.hpp"
#include "functional/ReactiveFunctions.hpp"
#include "tools/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace duckdb;
    using namespace google::protobuf;
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_DATA_NS;

    struct DuckDBStoreOptions {
        /**
         * Table for storing data
         */
        std::string table_name;

        /**
         * DB file path. If set, a standalone DuckDB instance will be created
         */
        std::string db_file_path;

        /**
         * Dimnesion for vector. Zero means no vector field.
         */
        size_t dimension = 0;

        /**
         * A flag to allow unknown metadata fields. Otherwise exception will be raised.
         */
        bool bypass_unknown_fields = true;

        /**
         * A flag to build standalone in-memory data store using DuckDb. This is for test purpose only.
         */
        bool in_memory = false;

        /**
         * A flag to initialize `CREATE OR REPLACE TABLE` syntax during startup. If set to false, `CREATE TABLE IF NOT EXISTS` syntax will be used
         */
        bool create_or_replace_table = false;
    };

    namespace details {
        static std::string make_mget_sql(
            const std::string& table_name,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const std::vector<std::string>& ids
        ) {
            assert_non_empty_range(ids, "ID list cannot be empty");
            std::string select_sql = "SELECT id, text";
            auto name_view = metadata_schema->fields() | std::views::transform(
                                 [](const MetadataFieldSchema& field)-> std::string {
                                     return field.name();
                                 });
            select_sql += name_view.empty() ? "" : ", " + StringUtils::JoinWith(name_view, ", ");
            select_sql += " FROM ";
            select_sql += table_name;
            select_sql += " WHERE id in (";
            for (int i = 0; i < ids.size(); ++i) {
                select_sql += "'";
                select_sql += ids[i];
                select_sql += "'";
                if (i < ids.size()) {
                    select_sql += ",";
                }
            }
            return select_sql + ");";
        }

        static std::string make_prepared_count_sql(const std::string& table_name) {
            return fmt::format("SELECT count(*) from {}", table_name);
        }

        static std::string make_create_table_sql(
            const std::string& table_name,
            const size_t dimmension,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            bool create_or_replace_table = false
        ) {
            auto create_table_sql = (create_or_replace_table ? "CREATE OR REPLACE TABLE " :  "CREATE TABLE IF NOT EXISTS ") + table_name + "(";
            std::vector<std::string> parts;
            parts.emplace_back("id UUID PRIMARY KEY");
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
            // delete_sql += StringUtils::JoinWith(ids, ", ");

            for (int i = 0; i < ids.size(); ++i) {
                delete_sql += "'";
                delete_sql += ids[i];
                delete_sql += "'";
                if (i < ids.size()-1) {
                    delete_sql += ",";
                }
            }
            delete_sql += ");";
            return delete_sql;
        }


        static void observe_query_result(QueryResult& query_result, const std::shared_ptr<MetadataSchema>& metadata_schema_, const auto& observer) {
            int count = 0;
            for (const auto& row: query_result) {
                Document document;
                document.set_id(row.GetValue<std::string>(0));
                document.set_text(row.GetValue<std::string>(1));
                for (int i = 0; i < metadata_schema_->fields_size(); i++) {
                    // plus 2 means skipping first two columns of  id and text in table
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
                count++;
                observer.on_next(document);
            }
            LOG_DEBUG("{} docs recalled", count);
            observer.on_completed();
        }

        static AsyncIterator<Document> conv_query_result_to_iterator(
            duckdb::unique_ptr<QueryResult> result,
            const MetadataSchemaPtr& metadata_schema_) {
            return rpp::source::create<Document>([metadata_schema_, result_ptr = std::move(result)](const auto& observer) {
                observe_query_result(*result_ptr, metadata_schema_, observer);
            });
        }

        static AsyncIterator<Document> conv_query_result_to_iterator(
                duckdb::unique_ptr<MaterializedQueryResult> result,
                const MetadataSchemaPtr& metadata_schema_) {
            return rpp::source::create<Document>([metadata_schema_, result_ptr = std::move(result)](const auto& observer) {
                observe_query_result(*result_ptr, metadata_schema_, observer);
            });
        }

        static void append_row_basic_fields(
            Appender& appender,
            const Document& doc,
            UpdateResult& update_result
        ) {
            // column of id
            const std::string new_id = StringUtils::GenerateUUIDString();
            update_result.add_returned_ids(new_id);
            appender.Append<>(new_id.c_str());
            // doc.set_id(new_id);
            update_result.add_returned_ids(new_id);

            // column of text
            // TODO escape text chars in case of sql injection
            appender.Append<>(doc.text().c_str());
        }

        static void append_row_column_value(Appender& appender, const Reflection* field_value_reflection, const FieldDescriptor* field_descriptor, const google::protobuf::Message& metadata_field_message) {

            if (field_descriptor) {
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
        }

        static void append_row_metadata_fields(
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            Appender& appender,
            const Document& doc,
            const bool bypass_unknown_fields
        ) {
            if (!metadata_schema || metadata_schema == EMPTY_METADATA_SCHEMA || metadata_schema->fields_size() == 0) {
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
//            assert_gte(metadata_size, metadata_schema->fields_size(), "Some metadata field(s) missing");


            auto defined_field_schema_map = DocumentUtils::ConvertToMetadataSchemaMap(metadata_schema);
            //std::unordered_map<std::string, const FieldDescriptor*> metadata_field_descriptor_map;


            std::unordered_map<std::string, int> metadata_field_name_index_map;

            for (int i = 0; i < metadata_size; i++) {
                const auto& metadata_field_message = reflection->GetRepeatedMessage(
                    doc,
                    metadata_field,
                    i);
                auto metadata_field_descriptor = metadata_field_message.GetDescriptor();
                auto* field_value_reflection = metadata_field_message.GetReflection();
                auto name_field_descriptor = metadata_field_descriptor->FindFieldByName("name");
                auto name = field_value_reflection->GetString(metadata_field_message, name_field_descriptor);

                metadata_field_name_index_map[name] = i;

                if (!bypass_unknown_fields) {
                    assert_true((known_field_names.contains(name)),
                                "Metadata cannot contain field not defined by schema. Problematic field is " + name);
                }
                known_field_names.erase(name);
            }

            assert_true(known_field_names.empty(),
                        "Some metadata fields not set: " + StringUtils::JoinWith(known_field_names, ","));


            // append columns according to the order in metadata schema, or DuckDB will complain with SQL errors.
            for (const auto& metadata_field_schema: metadata_schema->fields()) {
                int i = metadata_field_name_index_map[metadata_field_schema.name()];
                const auto& metadata_field_message = reflection->GetRepeatedMessage(
                        doc,
                        metadata_field,
                        i);

                auto metadata_field_descriptor = metadata_field_message.GetDescriptor();
                auto* field_value_reflection = metadata_field_message.GetReflection();
                auto* value_descriptor = metadata_field_descriptor->FindOneofByName("value");
//                auto* field_descriptor = value_descriptor->field(value_descriptor->index());
                //auto* field_descriptor = field_value_reflection->GetOneofFieldDescriptor(doc, value_descriptor);

                auto* value_field_descriptor = field_value_reflection->GetOneofFieldDescriptor(metadata_field_message, value_descriptor);

                append_row_column_value(appender, field_value_reflection, value_field_descriptor, metadata_field_message);
            }
        }
    }



    class BaseDuckDBStore : public virtual IDocStore {
        DuckDBStoreOptions options_;
        DuckDBPtr db_;
        std::shared_ptr<MetadataSchema> metadata_schema_;
        unique_ptr<PreparedStatement> prepared_count_all_statement_;
        // std::map<std::thread::id, Connection> connections_;
        // std::mutex g_i_mutex;

        Connection connection_;

    public:
        explicit BaseDuckDBStore(
            DuckDBPtr db,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const DuckDBStoreOptions& options
        ): options_(options),
           db_(std::move(db)),
           metadata_schema_(metadata_schema),
            connection_(*db_)
        {
            // LOG_DEBUG("Startup db at {}", options.db_file_path);
            assert_true(metadata_schema, "should provide schema");
            // assert_positive(options_.dimension, "dimension should be positive");
            assert_lt(options_.dimension, 10000, "dimension should be less than 10000");
            assert_true(!StringUtils::IsBlankString(options_.table_name), "table_name cannot be blank");

            const auto sql = details::make_create_table_sql(options_.table_name, options_.dimension, metadata_schema_, options.create_or_replace_table);
            LOG_DEBUG("create document table with SQL: {}", sql);

            const auto create_table_result = connection_.Query(sql);
            assert_query_ok(create_table_result);

            prepared_count_all_statement_ = connection_.Prepare(details::make_prepared_count_sql(options_.table_name));
            assert_prepared_ok(prepared_count_all_statement_, "Failed to prepare count statement");
        }


        [[nodiscard]] const DuckDBStoreOptions& GetOptions() const {
            return options_;
        }

        [[nodiscard]] std::shared_ptr<MetadataSchema> GetMetadataSchema() const override {
            return metadata_schema_;
        }

        [[nodiscard]] Connection GetConnection() {
            // const std::lock_guard<std::mutex> lock(g_i_mutex);
            // auto id = std::this_thread::get_id();
            // if (!connections_.contains(id)) {
            //     connections_.emplace(id, Connection(*db_));
            // }
            // return connections_.at(id);
            return std::move(Connection(*db_));
        }

        AsyncIterator<Document> MultiGetDocuments(const std::vector<std::string>& ids) override {
            if (ids.empty()) {
                return rpp::source::empty<Document>();
            }

            auto result = GetConnection().Query(details::make_mget_sql(options_.table_name, metadata_schema_, ids));
            assert_query_ok(result);
            return details::conv_query_result_to_iterator(std::move(result), metadata_schema_);
        }

        size_t CountDocuments() override {
            const auto result = prepared_count_all_statement_->Execute();
            assert_query_ok(result);
            for (const auto& row: *result) {
                return row.GetValue<uint32_t>(0);
            }
            throw InstinctException("Empty count result");
        }

        void AddDocuments(const AsyncIterator<Document>& documents_iterator, UpdateResult& update_result) override {
            std::vector<Document> docs;
            CollectVector(documents_iterator, docs);
            AddDocuments(docs, update_result);
        }

        virtual void AppendRows(Appender& appender, const std::vector<Document>& records, UpdateResult& update_result) = 0;

        void AddDocuments(const std::vector<Document>& records, UpdateResult& update_result) override {
            auto connection = GetConnection();
            connection.BeginTransaction();
            try {
                Appender appender(connection, options_.table_name);
                AppendRows(appender, records, update_result);
                appender.Close();
                connection.Commit();
            } catch (const duckdb::Exception& e) {
                connection.Rollback();
                LOG_ERROR("DuckDB Error, what()={}", e.what());
                throw InstinctException(e, "Failed to AddDocument due to DuckDB error.");
            } catch (...) {
                connection.Rollback();
                std::rethrow_exception(std::current_exception());
            }
        }

        virtual void AppendRow(Appender& appender, Document& doc, UpdateResult& update_result) = 0;

        void AddDocument(Document& doc) override {
            auto connection = GetConnection();
            connection.BeginTransaction();
            try {
                UpdateResult update_result;
                Appender appender(connection, options_.table_name);
                AppendRow(appender, doc, update_result);
                if (update_result.returned_ids_size() > 0) {
                    doc.set_id(update_result.returned_ids(0));
                }
                appender.Close();
                connection.Commit();
            } catch (const duckdb::Exception& e) {
                connection.Rollback();
                LOG_ERROR("DuckDB Error, what()={}", e.what());
                throw InstinctException(e, "Failed to AddDocument due to DuckDB error.");
            } catch (...) {
                connection.Rollback();
                std::rethrow_exception(std::current_exception());
            }

        }

        void DeleteDocuments(const std::vector<std::string>& ids, UpdateResult& update_result) override {
            auto connection = GetConnection();
            const auto sql = details::make_delete_sql(options_.table_name, ids);
            const auto result = connection.Query(sql);
            assert_query_ok(result);
            const auto xn = result->GetValue<int32_t>(0, 0);
            update_result.set_affected_rows(xn);
            update_result.mutable_returned_ids()->Add(ids.begin(), ids.end());
        }

        // using DuckDBStoreInternalPtr = std::shared_ptr<DuckDBStoreInternal>;
    };
}

#endif //BASEDUCKDBSTORE_HPP
