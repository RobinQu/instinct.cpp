//
// Created by RobinQu on 2024/4/2.
//

#ifndef BASEDUCKDBVECTORSTORE_HPP
#define BASEDUCKDBVECTORSTORE_HPP

#include <instinct/store/duckdb/base_duckdb_store.hpp>
#include <instinct/retrieval_global.hpp>
#include <instinct/model/embedding_model.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    namespace details {

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
            vector<duckdb::Value> vector_value;
            for (const float& f: embedding) {
                vector_value.push_back(duckdb::Value::FLOAT(f));
            }
            appender.Append(duckdb::Value::ARRAY(LogicalType::FLOAT, vector_value));

            // metadata fields
            append_row_metadata_fields(metadata_schema, appender, doc, bypass_unknown_fields);

            appender.EndRow();
        }
    }


    /**
     * Specialized DocStore that will embed input documents
     */
    class DuckDBDocWithEmbeddingStore final: public BaseDuckDBStore {
        EmbeddingsPtr embeddings_;
    public:
        DuckDBDocWithEmbeddingStore(
            const DuckDBPtr& db,
            const MetadataSchemaPtr &metadata_schema,
            const EmbeddingsPtr& embedding_model,
            const DuckDBStoreOptions &options
            )
            : BaseDuckDBStore(db, metadata_schema, options), embeddings_(embedding_model) {
        }

        [[nodiscard]] EmbeddingsPtr GetEmbedding() const {
            return embeddings_;
        }

        void AppendRows(Appender &appender, std::vector<Document> &records, UpdateResult &update_result) override {
            auto text_view = records | std::views::transform([](auto&& record) -> std::string {
                return record.text();
            });
            auto embeddings = embeddings_->EmbedDocuments({text_view.begin(), text_view.end()});
            assert_equal_size(embeddings, records, "Count of result embeddings is not equal to that of records");
            int affected_row = 0;
            for (int i=0;i<records.size();i++) {
                try {
                    details::append_row(GetMetadataSchema(), appender, records[i], embeddings[i], update_result, GetOptions().bypass_unknown_fields);
                    affected_row++;
                } catch (const InstinctException& e) {
                    update_result.add_failed_documents()->CopyFrom(records[i]);
                    LOG_WARN("AppendRows error: {}", e.what());
                }
            }
            update_result.set_affected_rows(affected_row);
        }

        void AppendRow(Appender &appender, Document &doc, UpdateResult &update_result) override {
            const auto embeddings = embeddings_->EmbedDocuments({doc.text()});
            details::append_row(GetMetadataSchema(), appender, doc, embeddings[0], update_result, GetOptions().bypass_unknown_fields);
        }
    };
}


#endif //BASEDUCKDBVECTORSTORE_HPP
