//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASEPROMPTTEMPLATE_HPP
#define INSTINCT_BASEPROMPTTEMPLATE_HPP

#include <utility>

#include "LLMGlobals.hpp"

#include "IPromptTemplate.hpp"
#include "functional/StepFunctions.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    struct PromptTemplateOptions {
        std::vector<std::string> input_keys = {};
        std::vector<std::string> output_keys = {DEFAULT_PROMPT_INPUT_KEY};

    };

    class BasePromptTemplate: public virtual IPromptTemplate, public BaseStepFunction {
        PromptTemplateOptions options_;
    public:
        explicit BasePromptTemplate(PromptTemplateOptions options) : options_(std::move(options)) {}

    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto prompt_value = FormatPrompt(input);
            input->PutMessage(DEFAULT_PROMPT_INPUT_KEY, prompt_value);
            return input;
        }

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return options_.input_keys;
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return options_.output_keys;
        }

    };

    using PromptTemplatePtr = std::shared_ptr<BasePromptTemplate>;
}

#endif //INSTINCT_BASEPROMPTTEMPLATE_HPP
