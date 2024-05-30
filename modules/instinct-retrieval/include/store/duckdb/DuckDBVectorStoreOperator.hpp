//
// Created by RobinQu on 2024/5/27.
//

#ifndef DUCKDBVECTORSTOREOPERATOR_HPP
#define DUCKDBVECTORSTOREOPERATOR_HPP
#include "BaseDuckDBStore.hpp"
#include "DuckDBDocStore.hpp"
#include "DuckDBVectorStore.hpp"
#include "../IVectorStoreOperator.hpp"
#include "store/VectorStoreMetadataDataMapper.hpp"

namespace INSTINCT_RETRIEVAL_NS {

    using EmbeddingModelSelector = std::function<EmbeddingsPtr(const std::string& instance_id, const MetadataSchemaPtr& metadata_schema)>;

    class DuckDBVectorStoreOperator final: public IVectorStoreOperator {
        DuckDBPtr duck_db_;
        MetadataSchemaPtr default_metadata_schema_;
        EmbeddingModelSelector embedding_model_selector_;
        std::mutex instances_mutex_;
        VectorStoreMetadataDataMapperPtr metadata_data_mapper_;
    public:
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
            instance_metadata.mutable_metadata_schema()->CopyFrom(*metadata_schema);
            assert_true(metadata_data_mapper_->InsertInstance(instance_metadata) == 1, "should have one instance inserted");
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
            options.instance_id = instance_id;
            ConfigureDuckDBOptions(options, embedding_model);
            assert_true(std::filesystem::exists(options.db_file_path), fmt::format("db path should exist at {}", options.db_file_path));
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
            return false;
        }


    private:
        void ConfigureDuckDBOptions(DuckDBStoreOptions& options, const EmbeddingsPtr& embedding_model) const {
            options.dimension = embedding_model->GetDimension();
            options.table_name = options.instance_id;
            options.create_or_replace_table = false;
            options.in_memory = false;
            options.bypass_unknown_fields = true;
        }
    };

    using DuckDBVectorStoreOperatorPtr = std::shared_ptr<DuckDBVectorStoreOperator>;
}

#endif //DUCKDBVECTORSTOREOPERATOR_HPP
