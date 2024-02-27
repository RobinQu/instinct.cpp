//
// Created by RobinQu on 2024/2/14.
//

#ifndef BASECHATPROMPTTEMPLATE_H
#define BASECHATPROMPTTEMPLATE_H
#include "CoreGlobals.hpp"
#include "ChatPromptValue.hpp"
#include "PromptTemplate.hpp"

namespace INSTINCT_CORE_NS {

class BaseChatPromptTemplate: public PromptTemplate {
public:
    virtual std::vector<MessageVariant> FormatMessages(const OptionDict& variables) = 0;

    PromptValueVairant FormatPrompt() override;

    PromptValueVairant FormatPrompt(const OptionDict& variables) override;

    std::string Format() override;

    std::string Format(const OptionDict& variables) override;
};

    inline PromptValueVairant BaseChatPromptTemplate::FormatPrompt() {
        return FormatPrompt({});
    }

    inline PromptValueVairant BaseChatPromptTemplate::FormatPrompt(const OptionDict& variables) {
        auto messages = FormatMessages(variables);
        return ChatPromptValue(messages);
    }

    inline std::string BaseChatPromptTemplate::Format() {
        return Format({});
    }

    inline std::string BaseChatPromptTemplate::Format(const OptionDict& variables) {
        const auto prompt = FormatPrompt(variables);
        return std::visit([](auto&& v) {
            return v.ToString();
        }, prompt);
    }

} // namespace INSTINCT_CORE_NS

#endif //BASECHATPROMPTTEMPLATE_H
