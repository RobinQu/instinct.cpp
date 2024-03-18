//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHATMEMORY_HPP
#define BASECHATMEMORY_HPP


#include "IChatMemory.hpp"
#include "LLMGlobals.hpp"
#include "chain/IChainContextAware.hpp"

namespace INSTINCT_LLM_NS {
    struct ChatMemoryOptions {
        std::string input_prompt_variable_key = DEFAULT_PROMPT_INPUT_KEY;
        std::string input_answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
        std::string output_memory_key = "chat_history";
    };

    class BaseChatMemory: public IChainContextAware, public IChatMemory{
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
        std::vector<std::string> GetInputKeys() override {
            // return {options_.input_answer_variable_key, options_.input_prompt_variable_key};
            return  {};
        }

        std::vector<std::string> GetOutputKeys() override {
            return {options_.output_memory_key};
        }

        /**
         * Add field of chat history to context
         * @param builder
         */
        void EnhanceContext(const ContextMutataorPtr& builder) override {
            LoadMemories(builder);
            builder->Commit();
        }

        void LoadMemories(const ContextMutataorPtr& bulder) override = 0;
    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
