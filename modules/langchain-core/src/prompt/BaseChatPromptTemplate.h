//
// Created by RobinQu on 2024/2/14.
//

#ifndef BASECHATPROMPTTEMPLATE_H
#define BASECHATPROMPTTEMPLATE_H
#include "BasePromptTemplate.h"
#include "CoreGlobals.h"


namespace LC_CORE_NS {

class BaseChatPromptTemplate: public BasePromptTemplate {
public:
    virtual std::vector<BaseMessagePtr> FormatMessages(const OptionDict& variables) = 0;

    PromptValuePtr FormatPrompt() override;

    PromptValuePtr FormatPrompt(const OptionDict& variables) override;

    std::string Format() override;

    std::string Format(const OptionDict& variables) override;
};

} // LC_CORE_NS

#endif //BASECHATPROMPTTEMPLATE_H
