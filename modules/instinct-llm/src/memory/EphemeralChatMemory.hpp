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
        EphemeralChatMemory() = default;

        void SaveMemory(const LLMChainContext& context) override {
            SetInputMessage(context, message_list_.add_messages());
        }

        void LoadMemories(const ChainContextBuilderPtr& builder) override {
            auto buffer = MessageUtils::CombineMessages(message_list_.messages());
            builder->Put(GetOptions().output_memory_key, buffer);
        }

    private:
        void SetInputMessage(const LLMChainContext& context, Message* msg) {
            auto& options = GetOptions();
            auto& input_key = options.input_prompt_variable_key;


            assert_true(context.values().contains(options.input_prompt_variable_key), "input prompt should be present in context with key :" + input_key);

            auto& v = context.values().at(input_key);
            assert_true(v.has_string_value(), "field " + input_key + " should be string value");
            msg->set_content(v.string_value());
            // TODO role name normalization
            msg->set_role("human");
        }
    };
}

#endif //EPHEMERALCHATMEMORY_HPP
