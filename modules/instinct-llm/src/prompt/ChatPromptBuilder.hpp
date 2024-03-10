//
// Created by RobinQu on 2024/3/10.
//

#ifndef CHATPROMPTBUILDER_HPP
#define CHATPROMPTBUILDER_HPP


#include "LLMGlobals.hpp"
#include <llm.pb.h>

namespace INSTINCT_LLM_NS {
    using namespace google::protobuf;

    enum MessageRole {
        kUnknown = 0,
        kAsistant,
        kHuman,
        kSystem,
        kFunction,
        kTool
    };

    using MessageRoleNameMapping = std::unordered_map<MessageRole, std::string>;

    class ChatPromptBulider {
        std::vector<std::pair<MessageRole,std::string>> values_;
        MessageRoleNameMapping role_name_mapping_;
    public:
        explicit ChatPromptBulider(MessageRoleNameMapping role_name_mapping)
            : role_name_mapping_(std::move(role_name_mapping)), values_() {
        }


        ChatPromptBulider* AddChatMessage(MessageRole role, const std::string& msg) {
            values_.emplace_back(role, msg);
            return this;
        }

        ChatPromptBulider* AddAIMessage(const std::string& msg) {
            return AddChatMessage(kAsistant, msg);
        }

        ChatPromptBulider* AddHumanMessage(const std::string& msg) {
            return AddChatMessage(kHuman, msg);
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
    using ChatPromptBuliderPtr = std::shared_ptr<ChatPromptBulider>;
}

#endif //CHATPROMPTBUILDER_HPP
