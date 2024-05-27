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
        std::filesystem::path data_root_;
        MetadataSchemaPtr& default_metadata_schema_;
        EmbeddingModelSelector embedding_model_selector_;
        std::mutex instances_mutext_;
        VectorStoreMeatdataDataMapperPtr meatdata_data_mapper_;
    public:
        DuckDBVectorStoreOperator(std::filesystem::path data_root, MetadataSchemaPtr &default_metadata_schema,
            EmbeddingModelSelector embedding_model_selector, VectorStoreMeatdataDataMapperPtr meatdata_data_mapper)
            : data_root_(std::move(data_root)),
              default_metadata_schema_(default_metadata_schema),
              embedding_model_selector_(std::move(embedding_model_selector)),
              meatdata_data_mapper_(std::move(meatdata_data_mapper)) {
        }

        VectorStorePtr CreateInstance(const std::string& instance_id, MetadataSchemaPtr metadata_schema) override {
            assert_not_blank(instance_id, "should have non-blank instance_id");
            DuckDBStoreOptions options;
             options.instance_id = instance_id;
            const auto instantce = meatdata_data_mapper_->GetInstance(instance_id);
            assert_true(!instantce, fmt::format("instance with id {} already exists", options.instance_id));

            const auto embedding_model = std::invoke(embedding_model_selector_, instance_id, metadata_schema);
            ConfigureDuckDBOptions(options, embedding_model);
            const auto instance = CreateDuckDBVectorStore(embedding_model, options, metadata_schema);
            VectorStoreInstanceMetadata instance_metadata;
            instance_metadata.set_instance_id(instance_id);
            instance_metadata.mutable_metadata_schema()->CopyFrom(*metadata_schema);
            assert_true(meatdata_data_mapper_->InsertInstance(instance_metadata) == 1, "should have one instance inserted");
            return instance;
        }

        VectorStorePtr CreateInstance(const std::string& instance_id) override {
            return CreateInstance(instance_id, default_metadata_schema_);
        }

        std::optional<VectorStorePtr> LoadInstance(const std::string &instance_id) override {
            assert_not_blank(instance_id, "should have non-blank instance_id");
            const auto instantce = meatdata_data_mapper_->GetInstance(instance_id);
            if (!instantce) {
                return std::nullopt;
            }
            auto metadata_schema = std::make_shared<MetadataSchema>(instantce->metadata_schema());
            const auto embedding_model = std::invoke(embedding_model_selector_, instance_id, metadata_schema);
            DuckDBStoreOptions options;
            options.instance_id = instance_id;
            ConfigureDuckDBOptions(options, embedding_model);
            assert_true(std::filesystem::exists(options.db_file_path), fmt::format("db path should exist at {}", options.db_file_path));
            return CreateDuckDBVectorStore(embedding_model, options, metadata_schema);
        }

        std::vector<std::string> ListInstances() override {
            const auto ids_view = meatdata_data_mapper_->ListInstances() | std::views::transform([](const VectorStoreInstanceMetadata& instance) {
                return instance.instance_id();
            });
            return {ids_view.begin(), ids_view.end()};
        }

        bool RemoveInstance(const std::string &instance_id) override {
            assert_not_blank(instance_id, "should have non-blank instance_id");
            if (const auto instantce = meatdata_data_mapper_->GetInstance(instance_id)) {
                assert_true(meatdata_data_mapper_->RemoveInstance(instance_id) == 1, "should have instantce meatdata deleted");
                return std::filesystem::remove(data_root_ / (instance_id + ".db"));
            }
            return false;
        }


    private:
        void ConfigureDuckDBOptions(DuckDBStoreOptions& options, const EmbeddingsPtr& embedding_model) const {
            options.db_file_path = data_root_ / (options.instance_id + ".db");
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
