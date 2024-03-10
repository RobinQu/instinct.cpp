//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H

#include "IPromptTemplate.hpp"
#include <string>
#include <vector>
#include "CoreGlobals.hpp"
#include "LLMGlobals.hpp"
#include <llm.pb.h>
#include <concepts>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class IChatPromptTemplate;
    using ChatPromptTemplatePtr = std::shared_ptr<IChatPromptTemplate>;
    using MessageLikeVariant = std::variant<Message, ChatPromptTemplatePtr>;

    // template<typename T>
    // requires std::derived_from<T, IChatPromptTemplate>
    // static ChatPromptTemplatePtr make_from_messages() {
    //     return std::make_shared<T>();
    // }




    class IChatPromptTemplate : public IPromptTemplate {
    public:
        // template<typename R>
        // requires RangeOf<R, MessageLikeVariant>
        // static ChatPromptTemplatePtr FromMessages(R&& range);

        // template<typename R>
        // requires RangeOf<R, MessagesLikeVariant>
        // explicit ChatPromptTemplate(R range): messages_({range.begin(), range.end()}) {}

        virtual MessageList FormatMessages(const LLMChainContext& variables) = 0;

        virtual ChatPromptValue FormtChatPrompt(const LLMChainContext& variables) = 0;

    };

    // template<typename R> requires RangeOf<R, MessageLikeVariant>
    // ChatPromptTemplatePtr ChatPromptTemplate::FromMessages(R&& range) {
    //     return ChatPromptTemplate(std::forward<R>(range));
    // }
    //
    // inline MessageVariants ChatPromptTemplate::FormatMessages(const OptionDict& variables) {
    //     auto message_view = messages_ | std::views::transform([&](const auto& m) {
    //         return std::visit([&](auto&& v) -> std::string {return v->Format(variables);}, m);
    //     });
    //     return {message_view.begin(), message_view.end()};
    // }


    namespace details {
        MessageList conv_message_alike_to_message_list(const MessageLikeVariant& mlv) {

        }
    }

} // core
// langchain

#endif //CHATPROMPTTEMPLATE_H
