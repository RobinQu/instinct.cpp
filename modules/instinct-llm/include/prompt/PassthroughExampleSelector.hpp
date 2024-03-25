//
// Created by RobinQu on 2024/2/14.
//

#ifndef PASSTHROUGHEXAMPLESELECTOR_H
#define PASSTHROUGHEXAMPLESELECTOR_H

#include "LLMGlobals.hpp"
#include "MutableExampleSelector.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    class PassthroughExampleSelector final : public MutableExampleSelector {
    public:
        explicit PassthroughExampleSelector(PromptTemplatePtr example_prompt_template)
            : MutableExampleSelector(std::move(example_prompt_template)) {
        }

        PromptExamples SelectExamples(const TemplateVariablesPtr & variables) override {
            return GetAllExamples();
        }
    };


} // namespace INSTINCT_CORE_NS

#endif //PASSTHROUGHEXAMPLESELECTOR_H
