//
// Created by RobinQu on 2024/1/10.
//

#include "FewShotPromptTemplate.h"
#include "CoreGlobals.h"
#include "tools/StringUtils.h"


namespace LC_CORE_NS {

    std::string FewShotPromptTemplate::Format() {
        return Format({});
    }

    std::string FewShotPromptTemplate::Format(const OptionDict& variables) {
        auto examples = example_selector_->SelectExamples(variables);
        auto example_strings_view = examples | std::views::transform([&](const auto& v) {
           return  example_prompt_template_->Format(v);
        });

        std::vector<std::string> parts = {prefix_};
        parts.insert(parts.end(), example_strings_view.begin(), example_strings_view.end());
        parts.push_back(suffix_);
        return langchian::core::StringUtils::JoinWith(parts, example_seperator_);
    }
} // core
// langchain