//
// Created by RobinQu on 2024/1/10.
//

#ifndef FEWSHOTPROMPTTEMPLATE_H
#define FEWSHOTPROMPTTEMPLATE_H

#include <string>
#include <vector>

#include "CoreGlobals.hpp"
#include "ExampleSelector.hpp"
#include "PlainPromptTemplate.hpp"
#include "PromptTemplate.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_CORE_NS {
    namespace details {
        static const auto DEFAULT_EXAMPLE_SEPERATOR = "\n\n";
    }

    class FewShotPromptTemplate : public StringPromptTemplate {
        ExampleSelector& example_selector_;
        PromptTemplate& example_prompt_template_;
        std::string prefix_;
        std::string suffix_;
        std::string example_seperator_;
    public:
        /**
         * \brief it's caller's responsiblilty to keep `example_selector_` and `example_prompt_template_` alive.
         * \param example_selector
         * \param example_prompt_template 
         * \param prefix 
         * \param suffix 
         * \param example_seperator 
         */
        FewShotPromptTemplate(
            ExampleSelector& example_selector,
            PromptTemplate& example_prompt_template,
                              std::string prefix, std::string suffix,
                              std::string example_seperator = details::DEFAULT_EXAMPLE_SEPERATOR)
            : example_selector_(example_selector),
              example_prompt_template_(example_prompt_template),
              prefix_(std::move(prefix)),
              suffix_(std::move(suffix)),
              example_seperator_(std::move(example_seperator)) {
        }

        std::string Format() override;

        std::string Format(const OptionDict& variables) override;
    };


    inline std::string FewShotPromptTemplate::Format() {
        return Format({});
    }

    inline std::string FewShotPromptTemplate::Format(const OptionDict& variables) {
        auto examples = example_selector_.SelectExamples(variables);
        auto example_strings_view = examples | std::views::transform([&](const auto& v) {
            return example_prompt_template_.Format(v);
        });

        std::vector<std::string> parts = {prefix_};
        parts.insert(parts.end(), example_strings_view.begin(), example_strings_view.end());
        parts.push_back(suffix_);
        return StringUtils::JoinWith(parts, example_seperator_);
    }
} // core
// langchain

#endif //FEWSHOTPROMPTTEMPLATE_H
