//
// Created by RobinQu on 2024/1/10.
//

#include "ChatPromptTemplate.h"

#include "ChatPromptValue.h"
#include "CoreGlobals.h"
#include <ranges>


namespace LC_CORE_NS {
    template<typename ... Arg>
    ChatPromptTemplatePtr ChatPromptTemplate::FromMessages(const Arg&... args) {
        std::vector<BaseMessagePromptTemplatePtr> messages;
        messages.insert(messages.end(), {args...});
        return std::make_shared<ChatPromptTemplate>(messages);
    }

    ChatPromptTemplate::ChatPromptTemplate(std::vector<BaseMessagePromptTemplatePtr> messages): messages_(std::move(messages)) {
    }

    std::vector<BaseMessagePtr> ChatPromptTemplate::FormatMessages(const OptionDict& variables) {
        auto message_view = messages_ | std::views::transform([&](const auto& m) {
            return m->Format(variables);
        });
        return {message_view.begin(), message_view.end()};
    }

} // core
// langchain