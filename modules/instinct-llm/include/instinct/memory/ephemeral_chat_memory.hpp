//
// Created by RobinQu on 2024/3/11.
//

#ifndef EPHEMERALCHATMEMORY_HPP
#define EPHEMERALCHATMEMORY_HPP

#include <instinct/memory/chat_memory.hpp>
#include <instinct/llm_global.hpp>
#include <instinct/prompt/message_utils.hpp>
#include <instinct/tools/assertions.hpp>


namespace INSTINCT_LLM_NS {
    /**
     * Chat history is stored in memory, which will be lost upon termination of current process
     */
    class EphemeralChatMemory final: public BaseChatMemory {
        MessageList message_list_;

    public:
        explicit EphemeralChatMemory(const ChatMemoryOptions& chat_memory_options = {}): BaseChatMemory(chat_memory_options) {}

        void SaveMemory(const PromptValue& prompt_value, const Generation& generation) override {
            if (prompt_value.has_chat()) {
                for(const auto& msg: prompt_value.chat().messages()) {
                    message_list_.add_messages()->CopyFrom(msg);
                }
            } else if(prompt_value.has_string()) {
                auto* msg = message_list_.add_messages();
                msg->set_content(prompt_value.string().text());
                // TODO role name normalization
                msg->set_role("human");
            }

            if (generation.has_message()) {
                message_list_.add_messages()->CopyFrom(generation.message());
            } else {
                auto* msg = message_list_.add_messages();
                msg->set_content(generation.text());
                // TODO role name normalization
                msg->set_role("assistant");
            }
        }

        MessageList LoadMemories() const override {
//            auto buffer = MessageUtils::CombineMessages(message_list_.messages());
//            context->PutPrimitive(GetOptions().output_memory_key, buffer);
            return message_list_;
        }

    };

}

#endif //EPHEMERALCHATMEMORY_HPP
