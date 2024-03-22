//
// Created by RobinQu on 2024/3/11.
//

#ifndef EPHEMERALCHATMEMORY_HPP
#define EPHEMERALCHATMEMORY_HPP

#include "BaseChatMemory.hpp"
#include "LLMGlobals.hpp"
#include "prompt/MessageUtils.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    /**
     * Chat history is stored in memory, which will be lost upon termination of current process
     */
    class EphemeralChatMemory: public BaseChatMemory {
        MessageList message_list_;

    public:
        explicit EphemeralChatMemory(const ChatMemoryOptions& chat_memory_options = {}): BaseChatMemory(chat_memory_options) {}

        void SaveMemory(const JSONContextPtr& context) override {
            auto& options = GetOptions();

            auto input_key = options.prompt_variable_key;
            auto output_key = options.answer_variable_key;

            if (context->Contains(input_key) && context->Contains(output_key)) {
                auto prompt_value = context->RequireMessage<PromptValue>(input_key);
                auto answer_value = context->RequireMessage<Generation>(output_key);


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

                if (answer_value.has_message()) {
                    message_list_.add_messages()->CopyFrom(answer_value.message());
                } else {
                    auto* msg = message_list_.add_messages();
                    msg->set_content(answer_value.text());
                    // TODO role name normalization
                    msg->set_role("assistant");
                }
                return;
            }

            throw InstinctException("Neither answer/prompt pair nor messages are present in context");
        }

        void LoadMemories(const JSONContextPtr& context) override {
            auto buffer = MessageUtils::CombineMessages(message_list_.messages());
            context->PutPrimitive(GetOptions().output_memory_key, buffer);
        }

    };

}

#endif //EPHEMERALCHATMEMORY_HPP
