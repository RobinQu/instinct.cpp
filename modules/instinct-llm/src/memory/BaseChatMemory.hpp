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
        std::string input_prompt_variable_key = "question";
        std::string output_memory_key = "chat_memory";
    };

    class BaseChatMemory: public IChainContextAware, public IChatMemory{
        ChatMemoryOptions options_;

    public:
        explicit BaseChatMemory(ChatMemoryOptions  options={})
            : options_(std::move(options)) {
        }

        const ChatMemoryOptions& GetOptions() {
            return options_;
        }



        std::vector<std::string> GetInputKeys() override {
            return {options_.input_prompt_variable_key};
        }

        std::vector<std::string> GetOutputKeys() override {
            return {options_.output_memory_key};
        }

        void SaveMemory(const LLMChainContext& context) override;

        /**
         * Add field of chat history to context
         * @param builder
         */
        void EnhanceContext(const ChainContextBuilderPtr& builder) override {
            LoadMemories(builder);
        }

        // void SaveMemory(const PromptValue& prompt_value, const Generation& generation) override;

        void LoadMemories(const ChainContextBuilderPtr& bulder) override = 0;
    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
