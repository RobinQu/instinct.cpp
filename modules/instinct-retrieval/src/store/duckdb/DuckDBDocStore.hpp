//
// Created by RobinQu on 2024/3/13.
//

#ifndef DUCKDBDOCSTORE_HPP
#define DUCKDBDOCSTORE_HPP
#include <duckdb.hpp>


#include "DuckDBStoreInternal.hpp"

#include "LLMGlobals.hpp"
#include "RetrievalGlobals.hpp"
#include "store/IDocStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace duckdb;

    namespace details {

        static void append_row(
                const std::shared_ptr<MetadataSchema>& metadata_schema,
                Appender& appender,
                Document& doc,
                UpdateResult& update_result,
                const bool bypass_unknown_fields
        ) {
            appender.BeginRow();

            // basic fields
            append_row_basic_fields(appender, doc, update_result);

            // metadata fields
            append_row_metadata_fields(metadata_schema, appender, doc, bypass_unknown_fields);

            appender.EndRow();
        }
    }


    class DuckDBDocStoreInternalAppender: public DuckDBInternalAppender {
        std::shared_ptr<MetadataSchema> metadata_schema_;
        bool bypass_unknonw_fields_;

    public:
        DuckDBDocStoreInternalAppender(std::shared_ptr<MetadataSchema> metadata_schema,
            const bool bypass_unknonw_fields)
            : metadata_schema_(std::move(metadata_schema)),
              bypass_unknonw_fields_(bypass_unknonw_fields) {

        }

        void AppendRow(Appender& appender, Document& doc, UpdateResult& update_result) override {
            details::append_row(metadata_schema_, appender, doc, update_result, bypass_unknonw_fields_);
        }

        void AppendRows(Appender& appender, std::vector<Document>& records, UpdateResult& update_result) override {
            int affected_row = 0;
            for (auto & record : records) {
                try {
                    details::append_row(metadata_schema_, appender, record, update_result, bypass_unknonw_fields_);
                    affected_row++;
                } catch (const InstinctException& e) {
                    update_result.add_failed_documents()->CopyFrom(record);
                    // TODO with better logging facilities
                    std::cerr << e.what() << std::endl;
                }
            }
            update_result.set_affected_rows(affected_row);
        }
    };

    class DuckDBDocStore final: public DuckDBStoreInternal {

    public:
        explicit DuckDBDocStore(
            const DuckDBStoreOptions& options,
            const std::shared_ptr<MetadataSchema>& metadata_schema)
            : DuckDBStoreInternal(
                std::make_shared<DuckDBDocStoreInternalAppender>(metadata_schema, options.bypass_unknonw_fields),
                options,
                metadata_schema
            ) {
            assert_true(!!metadata_schema, "should have provide valid metadata schema");
            assert_true(options.dimmension <=0);
        }
    };

    static DocStorePtr CreateDuckDBDocStore(const DuckDBStoreOptions& options,
            const std::shared_ptr<MetadataSchema>& metadata_schema = details::EMPTY_METADATA_SCHEMA) {
        return std::make_shared<DuckDBDocStore>(options, metadata_schema);
    }



}

#endif //DUCKDBDOCSTORE_HPP
