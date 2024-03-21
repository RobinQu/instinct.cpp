//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHATMEMORY_HPP
#define BASECHATMEMORY_HPP


#include "IChatMemory.hpp"
#include "LLMGlobals.hpp"
#include "chain/IChainContextAware.hpp"

namespace INSTINCT_LLM_NS {

    static const std::string DEFAULT_CHAT_HISTORY_KEY = "chat_history";

    struct ChatMemoryOptions {
        std::string input_prompt_variable_key = DEFAULT_PROMPT_INPUT_KEY;
        std::string input_answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
        std::string input_messages_list_variable_key = DEFAULT_MESSAGE_LIST_INPUT_KEY;
        std::string output_message_variable_key = DEFAULT_MESSAGE_OUTPUT_KEY;
        std::string output_memory_key = DEFAULT_CHAT_HISTORY_KEY;
    };

    class BaseChatMemory: public BaseStepFunction, public virtual IChatMemory {
        ChatMemoryOptions options_;
    public:
        BaseChatMemory()=delete;
        explicit BaseChatMemory(ChatMemoryOptions  options={})
            : options_(std::move(options)) {
        }

        const ChatMemoryOptions& GetOptions() {
            return options_;
        }

        /**
         * ChatMemory depends nothing when enhancing context. However, prompt and answer are required before SaveMemory, where extra validations are needed.
         * @return
         */
        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            // return {options_.input_answer_variable_key, options_.input_prompt_variable_key};
            return  {};
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return {options_.output_memory_key};
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto result = CreateJSONContext();
            LoadMemories(result);
            return result;
        }

        void LoadMemories(const JSONContextPtr& context) override = 0;
    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
