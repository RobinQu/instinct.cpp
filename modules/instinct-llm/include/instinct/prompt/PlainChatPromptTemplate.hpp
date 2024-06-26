//
// Created by RobinQu on 2024/3/9.
//

#ifndef PLAINCHATTEMPLATE_HPP
#define PLAINCHATTEMPLATE_HPP

#include <instinct/prompt/BaseChatPromptTemplate.hpp>
#include <instinct/LLMGlobals.hpp>
#include <instinct/prompt/MessageUtils.hpp>
#include <instinct/functional/JSONContextPolicy.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;



    class PlainChatPromptTemplate final: public BaseChatPromptTemplate {
        std::vector<MessageLikeVariant> messages_;

    public:
        explicit PlainChatPromptTemplate(const std::vector<MessageLikeVariant> &messages,
                          const PromptTemplateOptions &options = {})
                : BaseChatPromptTemplate(options), messages_(messages) {}


        MessageList FormatMessages(const TemplateVariablesPtr& variables) override {
            MessageList message_list;
            for (const auto& message_like: messages_) {
                if (std::holds_alternative<Message>(message_like)) {
                    message_list.add_messages()->CopyFrom(std::get<Message>(message_like));
                }
                if (std::holds_alternative<ChatPromptTemplatePtr>(message_like)) {
                    auto chat_prompt_template = std::get<ChatPromptTemplatePtr>(message_like);
                    auto part_list = chat_prompt_template->FormatMessages(variables);
                    message_list.MergeFrom(part_list);
                }
                // if message is history_placeholder, copy history messages from context variables
            }

            // format content field of each message
            for (int i=0;i<message_list.messages_size();++i) {
                auto* msg = message_list.mutable_messages(i);
                msg->set_content(MessageUtils::FormatString(msg->content(), variables));
            }
            return message_list;
        }

    };

    static PromptTemplatePtr CreatePlainChatPromptTemplate(const ChatPrompTeplateLiterals& literals, const MessageRoleNameMapping& mapping = DEFAULT_ROLE_NAME_MAPPING) {
        std::vector<MessageLikeVariant> messages;
        for(const auto&[role, content]: literals) {
            Message message;
            message.set_role(mapping.at(role));
            message.set_content(content);
            messages.emplace_back(message);
        }
        return std::make_shared<PlainChatPromptTemplate>(messages);
    }

}


#endif //PLAINCHATTEMPLATE_HPP
