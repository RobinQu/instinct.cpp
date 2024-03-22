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
        std::string input_prompt_variable_key = DEFAULT_PROMPT_INPUT_KEY;
        std::string input_answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
//        std::string input_messages_list_variable_key = DEFAULT_MESSAGE_LIST_INPUT_KEY;
//        std::string output_message_variable_key = DEFAULT_MESSAGE_OUTPUT_KEY;
        std::string output_memory_key = DEFAULT_CHAT_HISTORY_KEY;
    };

    class BaseChatMemory : public virtual IChatMemory {
        ChatMemoryOptions options_;
        StepFunctionPtr load_memory_function_;
        StepFunctionPtr save_memory_function_;
    public:
        BaseChatMemory() = delete;

        explicit BaseChatMemory(ChatMemoryOptions options = {})
                : options_(std::move(options)) {
            load_memory_function_ = std::make_shared<LambdaStepFunction>(
                    [&](const JSONContextPtr &context) {
                        LoadMemories(context);
                        return context;
                    },
                    std::vector<std::string> {},
                    std::vector<std::string> {options_.output_memory_key});

            save_memory_function_ = std::make_shared<LambdaStepFunction>([&](const JSONContextPtr &context) {
                this->SaveMemory(context);
                return context;
            });
        }

        const ChatMemoryOptions &GetOptions() {
            return options_;
        }

        [[nodiscard]] StepFunctionPtr AsLoadMemoryFunction() const {
            return load_memory_function_;
        }

        [[nodiscard]] StepFunctionPtr  AsSaveMemoryFunction() const {
            return save_memory_function_;
        }

        void LoadMemories(const JSONContextPtr &context) override = 0;

    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
