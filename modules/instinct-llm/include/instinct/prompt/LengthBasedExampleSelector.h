//
// Created by RobinQu on 2024/2/14.
//

#ifndef LENGTHBASEDPROMPTSELECTOR_H
#define LENGTHBASEDPROMPTSELECTOR_H

#include "LLMGlobals.hpp"

#include "MutableExampleSelector.hpp"
#include "tools/StringUtils.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using ExampleLengthFunction = std::function<size_t(const std::string&)>;

    namespace details {

        static ExampleLengthFunction DEFAULT_LENGTH_BASE_FUNCTION = [](const std::string& s) {return s.size();};
        static int DEFAULT_MAX_LENGTH = 25;
    }

    class LengthBasedExampleSelector final: public MutableExampleSelector {
        size_t max_length_;
        ExampleLengthFunction length_function_;
        std::vector<size_t> lengths_;

    public:
        LengthBasedExampleSelector(
            const PromptTemplatePtr& example_prompt_template,
            size_t max_length,
            ExampleLengthFunction length_function = details::DEFAULT_LENGTH_BASE_FUNCTION)
            : MutableExampleSelector(example_prompt_template),
                max_length_(max_length),
              length_function_(std::move(length_function)),
              lengths_() {
        }

        void AddExample(const PromptExample& example) override {
            MutableExampleSelector::AddExample(example);
            TemplateVariablesPtr variables = std::make_shared<TemplateVariables>(example);
            lengths_.push_back(length_function_(example_prompt_template_->Format(variables)));
        }

        PromptExamples SelectExamples(const TemplateVariablesPtr & variables) override {
            size_t remaining = max_length_;
            int i = 0;
            const size_t size = examples_.size();
            PromptExamples prompt_examples;
            while (remaining > 0 and i < size) {
                if (remaining - lengths_[i] < 0) {
                    break;
                }
                remaining -= lengths_[i];
                prompt_examples.push_back(examples_[i]);
                i++;
            }
            return prompt_examples;
        }
    };
} // namespace INSTINCT_CORE_NS

#endif //LENGTHBASEDPROMPTSELECTOR_H
