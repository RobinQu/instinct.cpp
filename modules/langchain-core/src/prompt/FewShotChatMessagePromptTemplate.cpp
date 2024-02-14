//
// Created by RobinQu on 2024/2/14.
//

#include "FewShotChatMessagePromptTemplate.h"

namespace LC_CORE_NS {
    std::vector<BaseMessagePtr> FewShotChatMessagePromptTemplate::FormatMessages(const OptionDict& variables) {
        auto examples = exmaple_selector_->SelectExamples(variables);
        auto example_messages_view = examples | std::views::transform([&](const auto& example) {
           return  example_prompt_template_->Format(example);
        });
        return {example_messages_view.begin(), example_messages_view.end()};
    }
} // LC_CORE_NS