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

        void SaveMemory(const ContextPtr& context) override {
            auto& options = GetOptions();
            auto& input_key = options.input_prompt_variable_key;
            auto& output_key = options.input_answer_variable_key;
            assert_true(context->values().contains(input_key), "input prompt should be present in context with key :" + input_key);
            assert_true(context->values().contains(output_key), "output answer should be present in context with key: " + output_key);

            auto& prompt_value = context->values().at(input_key);
            assert_true(prompt_value.has_string_value(), "field " + input_key + " should be string value");
            auto* msg = message_list_.add_messages();
            msg->set_content(prompt_value.string_value());
            // TODO role name normalization
            msg->set_role("human");

            auto& ansewr_value = context->values().at(output_key);
            msg = message_list_.add_messages();
            msg->set_content(ansewr_value.string_value());
            // TODO role name normalization
            msg->set_role("assistant");
        }

        void LoadMemories(const ContextMutataorPtr& builder) override {
            auto buffer = MessageUtils::CombineMessages(message_list_.messages());
            builder->Put(GetOptions().output_memory_key, buffer);
        }

    };

}

#endif //EPHEMERALCHATMEMORY_HPP
