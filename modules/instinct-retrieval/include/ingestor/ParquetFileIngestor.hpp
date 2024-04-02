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

    enum ParquetColumnType {
        kUnknownParquetColumn,
        kTextColumn,
        kVectorColumn,
        kMetadataColumn
    };

    struct ParquetColumnMapping {
        ParquetColumnType column_type;
        MetadataFieldSchema metadata_field_schema;
        int column_index;
    };

    class ParquetFileIngestor final: public BaseIngestor {
        std::string file_source;
        std::vector<ParquetColumnMapping> column_mapping;
    public:
        ParquetFileIngestor(std::string file_source, const std::vector<ParquetColumnMapping> &column_mapping)
            : file_source(std::move(file_source)),
              column_mapping(column_mapping) {
        }

        AsyncIterator<Document> Load() override {
            return rpp::source::create<Document>([&](const auto & observer) {
                duckdb::DuckDB duck_db(nullptr);
                duckdb::Connection conn(duck_db);
                const auto result = conn.Query(fmt::format("select * from read_parquest({});", file_source));
                if(details::check_query_ok(result)) {
                    for(const auto& row: *result) {
                        Document document;
                        for(int i=0; i<column_mapping.size();++i) {
                            const auto mapping = column_mapping[i];
                            const int column_idx = mapping.column_index;
                            if(mapping.column_type == kTextColumn) {
                                document.set_text(row.GetValue<std::string>(column_idx));
                                continue;
                            }
                            if (mapping.column_type == kMetadataColumn) {
                                auto* metadata_field = document.add_metadata();
                                metadata_field->set_name(mapping.metadata_field_schema.name());
                                switch (const auto field_schema = mapping.metadata_field_schema; field_schema.type()) {
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
                            observer.on_error(std::make_exception_ptr(InstinctException("unknown column type at index " + std::to_string(column_idx))));
                        }
                        observer.on_next(document);
                    }
                    observer.on_completed();
                } else {
                    observer.on_error(std::make_exception_ptr(InstinctException(result->GetError())));
                }
            });
        }
    };


    static IngestorPtr CreateParquetIngestor(const std::string& file_source, const std::vector<ParquetColumnMapping> &column_mapping) {
        return std::make_shared<ParquetFileIngestor>(file_source, column_mapping);
    }

    /**
     * 
     * @param file_source remote or local file source
     * @param mapping_string string literals that describes column mappings. e.g. "1:t,2:m:parent_doc_id:int64,3:m:source:varchar"
     * @return 
     */
    static IngestorPtr CreateParquetIngestor(const std::string& file_source, const std::string& mapping_string) {
        std::vector<ParquetColumnMapping> mappings;
        for(const auto& column: StringUtils::ReSplit(StringUtils::Trim(mapping_string), std::regex(","))) {
            if(StringUtils::IsBlankString(column)) continue;
            ParquetColumnMapping column_mapping;
            const auto column_parts = StringUtils::ReSplit(StringUtils::Trim(column), std::regex(":"));
            assert_gte(column_parts.size(), 2, "column definition string should contain at least two parts");
            const auto type_string = StringUtils::ToLower(column_parts[1]);

            column_mapping.column_index = std::stoi(column_parts[0]);
            if(type_string == "t") {
                column_mapping.column_type = kTextColumn;
            }
            if(type_string == "m") {
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

                column_mapping.metadata_field_schema = field_schema;
                column_mapping.column_type = kMetadataColumn;
            }
            mappings.push_back(column_mapping);
        }

        return CreateParquetIngestor(file_source, mappings);
    }

}

#endif //PARQUETFILEINGESTOR_HPP
