//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHATMEMORY_HPP
#define BASECHATMEMORY_HPP


#include "IChatMemory.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {

    static const std::string DEFAULT_CHAT_HISTORY_KEY = "chat_history";

    struct ChatMemoryOptions {
        std::string prompt_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
        std::string answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
        std::string output_memory_key = DEFAULT_CHAT_HISTORY_KEY;
    };

    struct SaveMemoryFunctionOptions {
        std::string prompt_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
        std::string answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    class BaseChatMemory : public virtual IChatMemory,
                           public virtual IConfigurable<ChatMemoryOptions>
            {
        ChatMemoryOptions options_;
        StepFunctionPtr load_memory_function_;
        StepFunctionPtr save_memory_function_;
    public:
        BaseChatMemory() = delete;

        explicit BaseChatMemory(ChatMemoryOptions options = {})
                : options_(std::move(options)) {
            load_memory_function_ = std::make_shared<LambdaStepFunction>(
                    [&](const JSONContextPtr &context) {
                        context->ProduceMessage(LoadMemories());
                        return context;
                    },
                    std::vector<std::string> {},
                    std::vector<std::string> {options_.output_memory_key});

        }

        void Configure(const ChatMemoryOptions &options) override {
            options_ = options;
        }

        const ChatMemoryOptions &GetOptions() {
            return options_;
        }

        StepFunctionPtr AsLoadMemoryFunction() {
            return load_memory_function_;
        }

        [[nodiscard]] StepFunctionPtr  AsSaveMemoryFunction(const SaveMemoryFunctionOptions& options) {
            return std::make_shared<LambdaStepFunction>([&](const JSONContextPtr &context) {
                auto generation = context->RequireMessage<Generation>(options.answer_variable_key);
                auto prompt = context->RequireMessage<PromptValue>(options.prompt_variable_key);
                this->SaveMemory(prompt, generation);
                return context;
            });
        }

    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
