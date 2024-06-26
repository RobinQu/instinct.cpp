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
        std::vector<std::string> input_keys = {DEFAULT_QUESTION_INPUT_OUTPUT_KEY};
//        std::vector<std::string> output_keys = {DEFAULT_QUESTION_INPUT_OUTPUT_KEY};
    };

    class BasePromptTemplate:
            public virtual IPromptTemplate,
            public BaseStepFunction {
        PromptTemplateOptions options_;
    public:
        explicit BasePromptTemplate(PromptTemplateOptions options) : options_(std::move(options)) {}

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            const TemplateVariablesPtr variables = CreateTemplateVariable();
            if (input->IsPrimitive()) {
                variables->emplace(options_.input_keys[0], input->RequirePrimitive<std::string>());
            } else {
                // only accepting primitive values
                for(const auto mapping_data = input->RequireMappingData(); const auto& [k,v]: mapping_data) {
                    if(v->IsPrimitive()) {
                        variables->emplace(k, v->GetValue());
                    } else {
                        LOG_WARN("discard data with key {} as it's non-primitive value", k);
                    }
                }
            }
            auto prompt_value = FormatPrompt(variables);
            input->ProduceMessage(prompt_value);
            return input;
        }

    };

    using PromptTemplatePtr = std::shared_ptr<BasePromptTemplate>;
}

#endif //INSTINCT_BASEPROMPTTEMPLATE_HPP
