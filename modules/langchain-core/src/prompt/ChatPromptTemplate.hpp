//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H

#include "PromptTemplate.hpp"
#include <string>
#include <vector>

#include "BaseChatPromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "Forwards.hpp"


LC_CORE_NS {
    class ChatPromptTemplate;
    using ChatPromptTemplatePtr = std::shared_ptr<ChatPromptTemplate>;


    class ChatPromptTemplate : public BaseChatPromptTemplate {
        std::vector<MessageLikeVariant> messages_;

    public:
        template<typename R>
        requires RangeOf<R, MessageLikeVariant>
        static ChatPromptTemplatePtr FromMessages(R&& range);

        template<typename R>
        requires RangeOf<R, MessageLikeVariant>
        explicit ChatPromptTemplate(R range): messages_({range.begin(), range.end()}) {}

        MessageVariants FormatMessages(const OptionDict& variables) override;

    };

    template<typename R> requires RangeOf<R, MessageLikeVariant>
    ChatPromptTemplatePtr ChatPromptTemplate::FromMessages(R&& range) {
        return ChatPromptTemplate(std::forward<R>(range));
    }

    inline MessageVariants ChatPromptTemplate::FormatMessages(const OptionDict& variables) {
        auto message_view = messages_ | std::views::transform([&](const auto& m) {
            return std::visit([&](auto&& v) -> std::string {return v->Format(variables);}, m);
        });
        return {message_view.begin(), message_view.end()};
    }
} // core
// langchain

#endif //CHATPROMPTTEMPLATE_H
