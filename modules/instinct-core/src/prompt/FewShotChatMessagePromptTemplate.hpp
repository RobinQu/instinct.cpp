//
// Created by RobinQu on 2024/2/14.
//

#ifndef FEWSHOTMESSAGEPROMPTTEMPLATE_H
#define FEWSHOTMESSAGEPROMPTTEMPLATE_H
#include "BaseChatPromptTemplate.hpp"
#include "ExampleSelector.hpp"
#include "MessagePromptTemplate.hpp"

namespace INSTINCT_CORE_NS {

class FewShotChatMessagePromptTemplate: public BaseChatPromptTemplate {

    MessagePromptTemplate& example_prompt_template_;
    ExampleSelector& exmaple_selector_;

public:
    /**
     * \brief it's caller's responsibility to keep example_prompt_template and exmaple_selector alive during lifecycle of FewShotChatMessagePromptTemplate
     * \param example_prompt_template
     * \param exmaple_selector
     */
    FewShotChatMessagePromptTemplate(
        MessagePromptTemplate& example_prompt_template,
        ExampleSelector& exmaple_selector)
        : example_prompt_template_(example_prompt_template),
          exmaple_selector_(exmaple_selector) {
    }

    MessageVariants FormatMessages(const OptionDict& variables) override;

};

    inline MessageVariants FewShotChatMessagePromptTemplate::FormatMessages(const OptionDict& variables) {
        auto examples = exmaple_selector_.SelectExamples(variables);
        auto example_messages_view = examples | std::views::transform([&](const auto& example) -> MessageVariant {
           return example_prompt_template_.Format(example);
        });
        return {example_messages_view.begin(), example_messages_view.end()};
    }

} // namespace INSTINCT_CORE_NS

#endif //FEWSHOTMESSAGEPROMPTTEMPLATE_H
