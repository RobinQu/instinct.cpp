//
// Created by RobinQu on 2024/2/28.
//

#ifndef RETRIEVALGLOBALS_HPP
#define RETRIEVALGLOBALS_HPP

#include <retrieval.pb.h>

#include <utility>
#include "CoreGlobals.hpp"
#include "LLMGlobals.hpp"

#define INSTINCT_RETRIEVAL_NS instinct::retrieval

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;
    struct RAGChainOptions {
        ChainOptions base_options = {};
        std::string context_output_key = DEFAULT_CONTEXT_OUTPUT_KEY;
        std::string condense_question_key = DEFAULT_STANDALONE_QUESTION_INPUT_KEY;
        int top_k = 10;
    };

    using MetadataSchemaPtr = std::shared_ptr<MetadataSchema>;

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
}


#endif //RETRIEVALGLOBALS_HPP
