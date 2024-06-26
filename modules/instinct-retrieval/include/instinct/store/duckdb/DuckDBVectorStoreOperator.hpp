//
// Created by RobinQu on 2024/5/27.
//

#ifndef DUCKDBVECTORSTOREOPERATOR_HPP
#define DUCKDBVECTORSTOREOPERATOR_HPP

#include <instinct/store/duckdb/BaseDuckDBStore.hpp>
#include <instinct/store/duckdb/DuckDBDocStore.hpp>
#include <instinct/store/duckdb/DuckDBVectorStore.hpp>
#include <instinct/store/IVectorStoreOperator.hpp>
#include <instinct/store/VectorStoreMetadataDataMapper.hpp>
#include <instinct/tools/HashUtils.hpp>
#include <instinct/tools/RandomUtils.hpp>

namespace INSTINCT_RETRIEVAL_NS {

    using EmbeddingModelSelector = std::function<EmbeddingsPtr(const std::string& instance_id, const MetadataSchemaPtr& metadata_schema)>;

    /**
     * Operator for DuckDB-based vector search, which uses a standalone table for each VectorStore instance
     */
    class DuckDBVectorStoreOperator final: public IVectorStoreOperator {
        DuckDBPtr duck_db_;
        MetadataSchemaPtr default_metadata_schema_;
        EmbeddingModelSelector embedding_model_selector_;
        VectorStoreMetadataDataMapperPtr metadata_data_mapper_;
    public:
        DuckDBVectorStoreOperator(
            const DuckDBPtr &db,
            const EmbeddingsPtr& embedding_model,
            const VectorStoreMetadataDataMapperPtr &metadata_data_mapper,
            const MetadataSchemaPtr &default_metadata_schema
        ): DuckDBVectorStoreOperator(
            db,
            [embedding_model](const std::string& instance_id, const MetadataSchemaPtr& metadata_schema) { return embedding_model; },
            metadata_data_mapper,
            default_metadata_schema
            ) {
            assert_true(duck_db_, "should provide DuckDB instance");
            assert_true(default_metadata_schema_, "should provide default metadata schema");
            assert_true(embedding_model_selector_, "should provide EmbeddingModelSelector");
            assert_true(metadata_data_mapper_, "should provide VectorStoreMetadataDataMapper");
        }

        DuckDBVectorStoreOperator(
            DuckDBPtr db,
            EmbeddingModelSelector embedding_model_selector,
            VectorStoreMetadataDataMapperPtr metadata_data_mapper,
            MetadataSchemaPtr  default_metadata_schema)
            : duck_db_(std::move(db)),
              default_metadata_schema_(std::move(default_metadata_schema)),
              embedding_model_selector_(std::move(embedding_model_selector)),
              metadata_data_mapper_(std::move(metadata_data_mapper)) {
        }

        VectorStorePtr CreateInstance(const std::string& instance_id, MetadataSchemaPtr metadata_schema) override {
            assert_true(metadata_schema, "should provide non-null metadata_schema");
            assert_not_blank(instance_id, "should have non-blank instance_id");
            DuckDBStoreOptions options;
             options.instance_id = instance_id;
            const auto instance = metadata_data_mapper_->GetInstance(instance_id);
            assert_true(!instance, fmt::format("instance with id {} already exists", options.instance_id));

            const auto embedding_model = std::invoke(embedding_model_selector_, instance_id, metadata_schema);
            ConfigureDuckDBOptions(options, embedding_model);
            const auto vdb_instance = CreateDuckDBVectorStore(duck_db_, embedding_model, options, metadata_schema);
            VectorStoreInstanceMetadata instance_metadata;
            instance_metadata.set_instance_id(instance_id);
            instance_metadata.set_embedding_table_name(TableNameForInstance_(instance_id));
            instance_metadata.mutable_metadata_schema()->CopyFrom(*metadata_schema);
            assert_true(metadata_data_mapper_->InsertInstance(instance_metadata), "should have one instance inserted");
            return vdb_instance;
        }

        VectorStorePtr CreateInstance(const std::string& instance_id) override {
            return CreateInstance(instance_id, default_metadata_schema_);
        }

        VectorStorePtr LoadInstance(const std::string &instance_id) override {
            assert_not_blank(instance_id, "should have non-blank instance_id");
            const auto instance = metadata_data_mapper_->GetInstance(instance_id);
            if (!instance) {
                return nullptr;
            }
            auto metadata_schema = std::make_shared<MetadataSchema>(instance->metadata_schema());
            const auto embedding_model = std::invoke(embedding_model_selector_, instance_id, metadata_schema);
            DuckDBStoreOptions options;
            // skip table creating as it should be already provisioned when calling this function
            options.bypass_table_check = true;
            options.instance_id = instance_id;
            ConfigureDuckDBOptions(options, embedding_model);
            return CreateDuckDBVectorStore(duck_db_, embedding_model, options, metadata_schema);
        }

        std::vector<std::string> ListInstances() override {
            const auto ids_view = metadata_data_mapper_->ListInstances() | std::views::transform([](const VectorStoreInstanceMetadata& instance) {
                return instance.instance_id();
            });
            return {ids_view.begin(), ids_view.end()};
        }

        bool RemoveInstance(const std::string &instance_id) override {
            if (const auto instance = LoadInstance(instance_id)) {
                instance->Destroy();
            }
            return metadata_data_mapper_->RemoveInstance(instance_id) == 1;
        }
    private:
        [[nodiscard]] std::string TableNameForInstance_(const std::string& instance_id) const {
            return "vs_" + HashUtils::HashForString<SHA256>(instance_id);
        }

        void ConfigureDuckDBOptions(DuckDBStoreOptions& options, const EmbeddingsPtr& embedding_model) const {
            options.dimension = embedding_model->GetDimension();
            options.table_name = TableNameForInstance_(options.instance_id);
            options.create_or_replace_table = false;
            options.in_memory = false;
            options.bypass_unknown_fields = true;
        }
    };

    static VectorStoreOperatorPtr CreateDuckDBStoreOperator(
        const DuckDBPtr &db,
        const EmbeddingsPtr& embedding_model,
        const VectorStoreMetadataDataMapperPtr &metadata_data_mapper,
        MetadataSchemaPtr default_metadata_schema = nullptr
        ) {
        if (!default_metadata_schema) {
            default_metadata_schema = CreateVectorStorePresetMetadataSchema();
        }
        assert_true(embedding_model, "should provide valid embedding model");
        return std::make_shared<DuckDBVectorStoreOperator>(db, embedding_model, metadata_data_mapper, default_metadata_schema);
    }

}

#endif //DUCKDBVECTORSTOREOPERATOR_HPP
