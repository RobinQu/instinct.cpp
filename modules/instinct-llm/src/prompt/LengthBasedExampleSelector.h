//
// Created by RobinQu on 2024/2/14.
//

#ifndef LENGTHBASEDPROMPTSELECTOR_H
#define LENGTHBASEDPROMPTSELECTOR_H

#include "LLMGlobals.hpp"
#include "CoreTypes.hpp"
#include "MutableExampleSelector.hpp"
#include "tools/StringUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    using ExampleLengthFunction = std::function<size_t(const std::string&)>;

    namespace details {

        static ExampleLengthFunction DEFAULT_LENGTH_BASE_FUNCTION = [](const std::string& s) {return s.size();};
        static int DEFAULT_MAX_LENGTH = 25;
    }

    class LengthBasedExampleSelector : public MutableExampleSelector {
        std::shared_ptr<LengthBasedExampleSelectorConfiguration> configuration_;
        ExampleLengthFunction length_function_;
        std::vector<size_t> lengths_;

    public:
        LengthBasedExampleSelector(
            const PromptTemplatePtr& example_prompt_template,
            std::shared_ptr<LengthBasedExampleSelectorConfiguration> configuration,
            ExampleLengthFunction length_function = details::DEFAULT_LENGTH_BASE_FUNCTION

            )
            : MutableExampleSelector(example_prompt_template), configuration_(std::move(configuration)),
              length_function_(std::move(length_function)),
              lengths_() {
        }

        void LengthBasedExampleSelector::AddExample(const PromptExample& example) override {
            MutableExampleSelector::AddExample(example);
            LLMChainContext ctx;
            for (const auto& entry: example.values()) {
                ctx.mutable_values()->insert(entry);
            }
            lengths_.push_back(length_function_(example_prompt_template_->Format(ctx)));
        }

        PromptExamples LengthBasedExampleSelector::SelectExamples(const LLMChainContext& variables) override {
            size_t remaining = configuration_->max_length();
            int i = 0;
            const size_t size = examples_.values_size();
            PromptExamples prompt_examples;
            while (remaining > 0 and i < size) {
                size_t len = remaining - lengths_[i];
                if (len < 0) {
                    break;
                }
                prompt_examples.add_values()
                        ->CopyFrom(examples_.values(i));
                i++;
            }
            return prompt_examples;
        }
    };
} // namespace INSTINCT_CORE_NS

#endif //LENGTHBASEDPROMPTSELECTOR_H
