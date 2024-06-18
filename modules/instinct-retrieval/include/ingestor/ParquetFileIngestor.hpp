//
// Created by RobinQu on 2024/4/2.
//

#ifndef PARQUETFILEINGESTOR_HPP
#define PARQUETFILEINGESTOR_HPP
#include <duckdb.hpp>
#include <store/duckdb/BaseDuckDBStore.hpp>
#include <utility>

#include "BaseIngestor.hpp"
#include "RetrievalGlobals.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    enum ParquetColumnType {
        kUnknownParquetColumn,
        kTextColumn,
        kVectorColumn,
        kMetadataColumn
    };

    struct ParquetColumnMapping {
        ParquetColumnType column_type = kUnknownParquetColumn;
        MetadataFieldSchema metadata_field_schema;
        int column_index = 0;
    };

    struct ParquetFileIngestorOptions {
        // used by naive ingestor to limit line count
        size_t limit;
    };

    class BaseParquetFileIngestor: public BaseIngestor {
        std::string file_source_;
        std::vector<ParquetColumnMapping> column_mapping_;
        std::string file_source_id_;
    public:
        BaseParquetFileIngestor(
            std::string file_source,
            const std::vector<ParquetColumnMapping> &column_mapping,
            const DocumentPostProcessor &document_post_processor ,
            std::string file_source_id)
            : BaseIngestor(document_post_processor),
            file_source_(std::move(file_source)),
              column_mapping_(column_mapping),
              file_source_id_(std::move(file_source_id)) {
        }

        virtual unique_ptr<MaterializedQueryResult> ReadParquet(Connection& conn, const std::string& file_source) = 0;

        AsyncIterator<Document> Load() override {
            return rpp::source::create<Document>([&](const auto & observer) {
                duckdb::DuckDB duck_db(nullptr);
                duckdb::Connection conn(duck_db);
                if(const auto result = ReadParquet(conn, file_source_); check_query_ok(result)) {
                    for(int i=0; const auto& row: *result) {
                        Document document = CreateNewDocument(
                            "",
                            ROOT_DOC_ID,
                            ++i,
                            StringUtils::IsBlankString(file_source_id_) ? file_source_ : file_source_id_
                        );
                        for(const auto&[column_type, metadata_field_schema, column_idx] : column_mapping_) {
                            if(column_type == kTextColumn) {
                                document.set_text(row.GetValue<std::string>(column_idx));
                                continue;
                            }
                            if (column_type == kMetadataColumn) {
                                auto* metadata_field = document.add_metadata();
                                metadata_field->set_name(metadata_field_schema.name());
                                switch (const auto field_schema = metadata_field_schema; field_schema.type()) {
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
                                        observer.on_error(std::make_exception_ptr(InstinctException("unknown field type for field named " + field_schema.name())));
                                }
                                continue;
                            }
                            observer.on_error(std::make_exception_ptr(InstinctException(fmt::format("unknown column type at index {}, column name {}, given column type {}",
                                std::to_string(column_idx),
                                metadata_field_schema.name(),
                                metadata_field_schema.type()
                                ))));
                        }
                        DocumentUtils::AddMissingPresetMetadataFields(document);
                        observer.on_next(document);
                    }
                    observer.on_completed();
                } else {
                    observer.on_error(std::make_exception_ptr(InstinctException(result->GetError())));
                }
            });
        }
    };

    /**
     * This ingestor will trigger a `SELECT *` against parquet file. A more delicate handling is needed for large parquet file.
     */
    class NaiveParquetFileIngestor final: public BaseParquetFileIngestor {
        ParquetFileIngestorOptions options_;
    public:
        NaiveParquetFileIngestor(const std::string &file_source,
            const std::vector<ParquetColumnMapping> &column_mapping,
            const DocumentPostProcessor &document_post_processor,
            const std::string& file_source_id,
            const ParquetFileIngestorOptions& options)
            : BaseParquetFileIngestor(file_source, column_mapping, document_post_processor, file_source_id), options_(options) {
        }

        unique_ptr<MaterializedQueryResult> ReadParquet(Connection &conn, const std::string &file_source) override {
            const auto sql_line = options_.limit > 0 ?
                fmt::format("select * from read_parquet('{}') limit {};", file_source, options_.limit):
                fmt::format("select * from read_parquet('{}');", file_source);
            LOG_DEBUG("Query SQL: {}", sql_line);
            return conn.Query(sql_line);
        }
    };


    static IngestorPtr CreateParquetIngestor(
        const std::string& file_source,
        const std::vector<ParquetColumnMapping> &column_mapping,
        const DocumentPostProcessor &document_post_processor,
        const std::string& parent_doc_id,
        const ParquetFileIngestorOptions& options) {
        return std::make_shared<NaiveParquetFileIngestor>(file_source, column_mapping, document_post_processor, parent_doc_id, options);
    }

    /**
     * 
     * @param file_source remote or local file source
     * @param mapping_string string literals that describes column mappings. e.g. "0:t,1:m:parent_doc_id:int64,3:m:source:varchar"
     * @param document_post_processor
     * @param file_source_id
     * @param options
     * @return 
     */
    static IngestorPtr CreateParquetIngestor(const std::string& file_source, const std::string& mapping_string, const ParquetFileIngestorOptions& options = {}, const DocumentPostProcessor &document_post_processor = nullptr, const std::string& file_source_id = "") {
        std::vector<ParquetColumnMapping> mappings;

        bool found_text;
        for(const auto& column: StringUtils::ReSplit(StringUtils::Trim(mapping_string), std::regex(","))) {
            if(StringUtils::IsBlankString(column)) continue;
            ParquetColumnMapping column_mapping;
            const auto column_parts = StringUtils::ReSplit(StringUtils::Trim(column), std::regex(":"));
            assert_gte(column_parts.size(), 2, "column definition string should contain at least two parts");
            const auto type_string = StringUtils::ToLower(column_parts[1]);

            column_mapping.column_index = std::stoi(column_parts[0]);
            if(type_string == "t" || type_string == "text" || type_string == "txt") {
                found_text = true;
                column_mapping.column_type = kTextColumn;
            }
            if(type_string == "m" || type_string == "metadata") {
                assert_gte(column_parts.size(), 4, "definition for metadata field should contain exactly four parts.");
                MetadataFieldSchema field_schema;
                field_schema.set_name(column_parts[2]);

                if(column_parts[3] == "int32") {
                    field_schema.set_type(INT32);
                }

                if(column_parts[3] == "int64") {
                    field_schema.set_type(INT64);
                }

                if(column_parts[3] == "float") {
                    field_schema.set_type(FLOAT);
                }

                if(column_parts[3] == "double") {
                    field_schema.set_type(DOUBLE);
                }

                if(column_parts[3] == "bool") {
                    field_schema.set_type(BOOL);
                }

                if(column_parts[3] == "varchar") {
                    field_schema.set_type(VARCHAR);
                }

                assert_true(field_schema.type() != PrimitiveType::Unknown, "should have specified correct field type");

                column_mapping.metadata_field_schema = field_schema;
                column_mapping.column_type = kMetadataColumn;
            }
            mappings.push_back(column_mapping);
        }

        assert_true(found_text, "should have text column specified");

        return CreateParquetIngestor(file_source, mappings, document_post_processor, file_source_id, options);
    }

}

#endif //PARQUETFILEINGESTOR_HPP
