//
// Created by RobinQu on 2024/3/10.
//

#ifndef CHATPROMPTBUILDER_HPP
#define CHATPROMPTBUILDER_HPP


#include "LLMGlobals.hpp"
#include <llm.pb.h>

#include "PlainChatTemplate.hpp"

namespace INSTINCT_LLM_NS {
    using namespace google::protobuf;

    enum MessageRole {
        kUnknown = 0,
        kAsisstant,
        kHuman,
        kSystem,
        kFunction,
        kTool
    };

    using MessageRoleNameMapping = std::unordered_map<MessageRole, std::string>;


    template<typename Impl>
    class BaseChatPromptBulider {
    protected:
        std::vector<std::pair<MessageRole,std::string>> values_;
        MessageRoleNameMapping role_name_mapping_;
    public:
        explicit BaseChatPromptBulider(MessageRoleNameMapping role_name_mapping)
            : role_name_mapping_(std::move(role_name_mapping)), values_() {
        }

        Impl* AddChatMessage(MessageRole role, const std::string& msg) {
            values_.emplace_back(role, msg);
            // return dynamic_cast<Impl*>(this);
            return static_cast<Impl*>(this);
        }

        Impl* AddAIMessage(const std::string& msg) {
            return AddChatMessage(kAsisstant, msg);
        }

        Impl* AddSystemMessage(const std::string& msg) {
            return AddChatMessage(kSystem, msg);
        }

        Impl* AddHumanMessage(const std::string& msg) {
            return AddChatMessage(kHuman, msg);
        }
    };

    class ChatPromptBulider: public BaseChatPromptBulider<ChatPromptBulider> {
    public:
        explicit ChatPromptBulider(MessageRoleNameMapping role_name_mapping)
            : BaseChatPromptBulider<ChatPromptBulider>(std::move(role_name_mapping)) {
        }

        PromptValue Build() {
            PromptValue pv;
            auto chat_prompt_value = pv.mutable_chat();
            // ChatPromptValue chat_prompt_value;
            for(const auto& [role,content]: values_) {
                auto* msg = chat_prompt_value->add_messages();
                msg->set_role(role_name_mapping_[role]);
                msg->set_content(content);
            }
            return pv;
        }

        PromptValue* Build(Arena* arena) {
            auto* pv = Arena::Create<PromptValue>(arena);
            auto chat_prompt_value = pv->mutable_chat();
            for(const auto& [role,content]: values_) {
                auto* msg = chat_prompt_value->add_messages();
                msg->set_role(role_name_mapping_[role]);
                msg->set_content(content);
            }
            return pv;
        }
    };

    class ChatPromptTemplateBulider: public BaseChatPromptBulider<ChatPromptTemplateBulider> {
    public:
        explicit ChatPromptTemplateBulider(MessageRoleNameMapping role_name_mapping)
            : BaseChatPromptBulider<ChatPromptTemplateBulider>(std::move(role_name_mapping)) {
        }

        PromptTemplatePtr Build() {
            std::vector<MessageLikeVariant> messages;
            for(const auto& [role,content]: values_) {
                Message message;
                message.set_content(content);
                message.set_role(role_name_mapping_[role]);
                messages.emplace_back(message);
            }
            return std::make_shared<PlainChatTemplate>(messages);
        }
    };

    using ChatPromptBuliderPtr = std::shared_ptr<ChatPromptBulider>;
    using ChatPromptTmeplateBuilderPtr = std::shared_ptr<ChatPromptTemplateBulider>;
}

#endif //CHATPROMPTBUILDER_HPP
