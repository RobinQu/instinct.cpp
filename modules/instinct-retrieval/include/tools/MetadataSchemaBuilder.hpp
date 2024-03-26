//
// Created by RobinQu on 2024/3/26.
//

#ifndef METADATASCHEMABUILDER_HPP
#define METADATASCHEMABUILDER_HPP

#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class MetadataSchemaBuilder final {
        MetadataSchemaPtr ptr_;
    public:

        static std::shared_ptr<MetadataSchemaBuilder> Create(const MetadataSchemaPtr &ptr = nullptr) {
            if (ptr) {
                return std::make_shared<MetadataSchemaBuilder>(ptr);
            }
            return std::make_shared<MetadataSchemaBuilder>(std::make_shared<MetadataSchema>());
        }

        explicit MetadataSchemaBuilder(MetadataSchemaPtr  ptr) : ptr_(std::move(ptr)) {}

        auto DefineString(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(VARCHAR);
            return this;
        }

        auto DefineInt32(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(INT32);
            return this;
        }

        auto DefineInt64(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(INT64);
            return this;
        }

        auto DefineFloat(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(FLOAT);
            return this;
        }
        auto DefineDouble(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(DOUBLE);
            return this;
        }

        auto DefineBool(const std::string& name) {
            auto field = ptr_->add_fields();
            field->set_name(name);
            field->set_type(BOOL);
            return this;
        }

        [[nodiscard]] MetadataSchemaPtr Build() const {
            return ptr_;
        }
    };


    static MetadataSchemaPtr CreatePresetMetdataSchema() {
        auto builder = MetadataSchemaBuilder::Create();
        builder->DefineString(METADATA_SCHEMA_PARENT_DOC_ID_KEY);
        builder->DefineInt32(METADATA_SCHEMA_PAGE_NO_KEY);
        builder->DefineString(METADATA_SCHEMA_FILE_SOURCE_KEY);
        return builder->Build();
    }
}

#endif //METADATASCHEMABUILDER_HPP
