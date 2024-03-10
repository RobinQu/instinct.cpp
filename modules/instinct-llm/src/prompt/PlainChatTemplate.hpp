//
// Created by RobinQu on 2024/3/9.
//

#ifndef PLAINCHATTEMPLATE_HPP
#define PLAINCHATTEMPLATE_HPP

#include "IChatPromptTemplate.hpp"
#include "LLMGlobals.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    class PlainChatTemplate: public IChatPromptTemplate {
        std::vector<MessageLikeVariant> messages_;

    public:
        explicit PlainChatTemplate(std::vector<MessageLikeVariant> messages)
            : messages_(std::move(messages)) {
        }

        PromptValue FormatPrompt(const LLMChainContext& variables) override {
            PromptValue prompt_value;
            prompt_value.mutable_chat()->CopyFrom(FormtChatPrompt(variables));
            return prompt_value;
        }

        std::string Format(const LLMChainContext& variables) override {
            const auto message_list = FormatMessages(variables);
            return MessageUtils::CombineMessages(message_list.messages());
        }

        StringPromptValue FormatStringPrompt(const LLMChainContext& variables) override {
            StringPromptValue spv;
            spv.set_text(Format(variables));
            return spv;
        }


        MessageList FormatMessages(const LLMChainContext& variables) override {
            MessageList message_list;
            for (const auto& message_like: messages_) {
                auto list = std::visit(details::conv_message_alike_to_message_list, message_like);
                message_list.MergeFrom(list);
            }

            // format content field of each message
            for (int i=0;i<message_list.messages_size();++i) {
                auto* msg = message_list.mutable_messages(i);
                msg->set_content(MessageUtils::FormatString(msg->content(), variables));
            }
            return message_list;
        }

        ChatPromptValue FormtChatPrompt(const LLMChainContext& variables) override {
            ChatPromptValue cpv;
            auto message_list = FormatMessages(variables);
            for (const auto&msg: message_list.messages()) {
                cpv.add_messages()->MergeFrom(msg);
            }
            return cpv;
        }
    };
}


#endif //PLAINCHATTEMPLATE_HPP
