//
// Created by RobinQu on 2024/3/9.
//

#ifndef PLAINCHATTEMPLATE_HPP
#define PLAINCHATTEMPLATE_HPP

#include "IChatPromptTemplate.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    class PlainChatTemplate: public IChatPromptTemplate {
        std::vector<MessageLikeVariant> messages_;

    public:
        explicit PlainChatTemplate(std::vector<MessageLikeVariant> messages)
            : messages_(std::move(messages)) {
        }

        MessageList FormatMessages(const LLMChainContext& variables) override {
            MessageList message_list;
            for (const auto& message_like: messages_) {
                auto list = std::visit(details::conv_message_alike_to_message_list, message_like);
                message_list.MergeFrom(list);
            }
            return message_list;
        }

        ChatPromptValue FormtChatPrompt(const LLMChainContext& variables) override {
            ChatPromptValue cpv;
            auto message_list = FormatMessages(variables);
            for (const auto&msg: message_list) {
                cpv.add_messages()->MergeFrom(msg);
            }
            return cpv;
        }
    };
}


#endif //PLAINCHATTEMPLATE_HPP
