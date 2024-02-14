//
// Created by RobinQu on 2024/2/14.
//

#include "BaseChatPromptTemplate.h"

#include "ChatPromptValue.h"

namespace LC_CORE_NS {
    PromptValuePtr BaseChatPromptTemplate::FormatPrompt() {
        return FormatPrompt({});
    }

    PromptValuePtr BaseChatPromptTemplate::FormatPrompt(const OptionDict& variables) {
        auto messages = FormatMessages(variables);
        return std::make_shared<ChatPromptValue>(messages);
    }

    std::string BaseChatPromptTemplate::Format() {
        return Format({});
    }

    std::string BaseChatPromptTemplate::Format(const OptionDict& variables) {
        return FormatPrompt(variables)->ToString();
    }
} // LC_CORE_NS