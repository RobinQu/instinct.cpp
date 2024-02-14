//
// Created by RobinQu on 2024/2/14.
//

#ifndef FEWSHOTMESSAGEPROMPTTEMPLATE_H
#define FEWSHOTMESSAGEPROMPTTEMPLATE_H
#include "BaseChatPromptTemplate.h"
#include "BaseExmapleSelector.h"
#include "BaseMessagePromptTemplate.h"

namespace LC_CORE_NS {

class FewShotChatMessagePromptTemplate: public BaseChatPromptTemplate {

    BaseMessagePromptTemplatePtr example_prompt_template_;
    BaseExampleSelectorPtr exmaple_selector_;

public:
    FewShotChatMessagePromptTemplate(BaseMessagePromptTemplatePtr example_prompt_template,
        BaseExampleSelectorPtr exmaple_selector)
        : example_prompt_template_(std::move(example_prompt_template)),
          exmaple_selector_(std::move(exmaple_selector)) {
    }

    std::vector<BaseMessagePtr> FormatMessages(const OptionDict& variables) override;
};

} // LC_CORE_NS

#endif //FEWSHOTMESSAGEPROMPTTEMPLATE_H
