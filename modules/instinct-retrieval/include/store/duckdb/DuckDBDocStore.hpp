//
// Created by RobinQu on 2024/3/13.
//

#ifndef DUCKDBDOCSTORE_HPP
#define DUCKDBDOCSTORE_HPP
#include <duckdb.hpp>


#include "BaseDuckDBStore.hpp"

#include "LLMGlobals.hpp"
#include "RetrievalGlobals.hpp"
#include "store/IDocStore.hpp"
#include "tools/MetadataSchemaBuilder.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;
    using namespace INSTINCT_CORE_NS;
    using namespace duckdb;

    namespace details {

        static void append_row(
                const std::shared_ptr<MetadataSchema>& metadata_schema,
                Appender& appender,
                const Document& doc,
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


    /**
     * Valillan storage for documents backed by DuckDB instance
     */
    class DuckDBDocStore final: public BaseDuckDBStore {
    public:
        explicit DuckDBDocStore(
            const DuckDBPtr& db,
            const std::shared_ptr<MetadataSchema>& metadata_schema,
            const DuckDBStoreOptions& options
            )
            : BaseDuckDBStore(
                db,
                metadata_schema,
                options
            )  {
            assert_true(metadata_schema, "should have provide valid metadata schema");
            assert_true(options.dimension <= 0);
        }


        void AppendRows(Appender &appender, const std::vector<Document> &records, UpdateResult &update_result) override {
            int affected_row = 0;
            for (auto & record : records) {
                try {
                    details::append_row(GetMetadataSchema(), appender, record, update_result, GetOptions().bypass_unknown_fields);
                    affected_row++;
                } catch (const InstinctException& e) {
                    update_result.add_failed_documents()->CopyFrom(record);
                    // TODO with better logging facilities
                    std::cerr << e.what() << std::endl;
                }
            }
            update_result.set_affected_rows(affected_row);

        }

        void AppendRow(Appender &appender, Document &doc, UpdateResult &update_result) override {
            details::append_row(GetMetadataSchema(), appender, doc, update_result, GetOptions().bypass_unknown_fields);
        }
    };

    static DocStorePtr CreateDuckDBDocStore(
        const DuckDBPtr& db,
        const DuckDBStoreOptions& options,
        std::shared_ptr<MetadataSchema> metadata_schema = nullptr
    ) {
        if (!metadata_schema) {
            metadata_schema = CreateDocStorePresetMetadataSchema();
        }
        return std::make_shared<DuckDBDocStore>(db, metadata_schema, options);
    }

    static DocStorePtr CreateDuckDBDocStore(
        const DuckDBStoreOptions& options,
        const std::shared_ptr<MetadataSchema>& metadata_schema = nullptr
    ) {
        if (options.in_memory) {
            return CreateDuckDBDocStore(std::make_shared<DuckDB>(nullptr), options, metadata_schema);
        }
        return CreateDuckDBDocStore(std::make_shared<DuckDB>(options.db_file_path), options, metadata_schema);
    }


}

#endif //DUCKDBDOCSTORE_HPP
