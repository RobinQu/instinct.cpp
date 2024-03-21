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

    struct FewShotPromptTemplateOptions: public PromptTemplateOptions{
        std::string prefix;
        std::string suffix;
    };


    class FewShotPromptTemplate final : public StringPromptTemplate {
        ExmapleSelectorPtr example_selector_;
        PromptTemplatePtr example_prompt_template_;
        std::string prefix_;
        std::string suffix_;
        std::string example_seperator_;
    public:
        FewShotPromptTemplate(
                ExmapleSelectorPtr example_selector,
                PromptTemplatePtr example_prompt_template,
                const FewShotPromptTemplateOptions& options = {}
        ):
                StringPromptTemplate(options),
                example_selector_(std::move(example_selector)),
                example_prompt_template_(std::move(example_prompt_template)),
                prefix_(options.prefix),
                suffix_(options.suffix)
                {}

        std::string Format(const JSONContextPtr & variables) override {
            auto examples = example_selector_->SelectExamples(variables);
            auto example_strings_view = examples.values() | std::views::transform([&](const auto& v) -> std::string {
                return example_prompt_template_->Format(variables);
            });
            std::vector parts = {prefix_};
            parts.insert(parts.end(), example_strings_view.begin(), example_strings_view.end());
            parts.push_back(suffix_);
            return StringUtils::JoinWith(parts, StringUtils::GetWithDefault(example_seperator_, details::DEFAULT_EXAMPLE_SEPERATOR));
        }
    };
}

#endif //FEWSHOTPROMPTTEMPLATE_H
