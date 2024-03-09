//
// Created by RobinQu on 2024/1/10.
//

#ifndef FEWSHOTPROMPTTEMPLATE_H
#define FEWSHOTPROMPTTEMPLATE_H

#include <string>
#include <vector>

#include "CoreGlobals.hpp"
#include "IExampleSelector.hpp"
#include "PlainPromptTemplate.hpp"
#include "IPromptTemplate.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    namespace details {
        static const auto DEFAULT_EXAMPLE_SEPERATOR = "\n\n";
    }

    class FewShotPromptTemplate final : public StringPromptTemplate {
        ExmapleSelectorPtr example_selector_;
        PromptTemplatePtr example_prompt_template_;
        std::shared_ptr<FewShotPromptTemplateConfiguration> configuration_;
    public:
        FewShotPromptTemplate(ExmapleSelectorPtr example_selector, PromptTemplatePtr example_prompt_template,
            std::shared_ptr<FewShotPromptTemplateConfiguration> configuration)
            : example_selector_(std::move(example_selector)),
              example_prompt_template_(std::move(example_prompt_template)),
              configuration_(std::move(configuration)) {
        }

        std::string Format(const LLMChainContext& variables) override {
            auto examples = example_selector_->SelectExamples(variables);
            auto example_strings_view = examples.values() | std::views::transform([&](const auto& v) -> std::string {
                LLMChainContext context;
                return example_prompt_template_->Format(context);
            });
            //
            std::vector parts = {configuration_->prefix()};
            parts.insert(parts.end(), example_strings_view.begin(), example_strings_view.end());
            parts.push_back(configuration_->suffix());
            return StringUtils::JoinWith(parts, StringUtils::GetWithDefault(configuration_->example_seperator(), details::DEFAULT_EXAMPLE_SEPERATOR));
        }
    };
}

#endif //FEWSHOTPROMPTTEMPLATE_H
