//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASECHATMEMORY_HPP
#define BASECHATMEMORY_HPP


#include <instinct/llm_global.hpp>

namespace INSTINCT_LLM_NS {

    class IChatMemory {
    public:
        IChatMemory()=default;
        virtual ~IChatMemory() = default;
        IChatMemory(const IChatMemory&)=delete;
        IChatMemory(IChatMemory&&)=delete;

        virtual void SaveMemory(const PromptValue& prompt_value, const Generation& generation) = 0;
        [[nodiscard]] virtual MessageList LoadMemories() const = 0;
    };

    static const std::string DEFAULT_CHAT_HISTORY_KEY = "chat_history";

    struct ChatMemoryOptions {
        std::string prompt_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
        std::string answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
        std::string output_memory_key = DEFAULT_CHAT_HISTORY_KEY;
    };

    struct SaveMemoryFunctionOptions {
        bool is_question_string;
        std::string prompt_variable_key = DEFAULT_QUESTION_INPUT_OUTPUT_KEY;
        std::string answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    class BaseChatMemory : public virtual IChatMemory,
                           public virtual IConfigurable<ChatMemoryOptions>
            {
        ChatMemoryOptions options_;
        StepFunctionPtr load_memory_function_{};
        StepFunctionPtr save_memory_function_{};
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

        [[nodiscard]] StepFunctionPtr AsSaveMemoryFunction(const SaveMemoryFunctionOptions& options) {
            return std::make_shared<LambdaStepFunction>([&, options](const JSONContextPtr &context) {
                auto mapping_data = context->RequireMappingData();
                auto generation = mapping_data.at(options.answer_variable_key)->RequireMessage<Generation>();
                if(options.is_question_string) {
                    auto question = mapping_data.at(options.prompt_variable_key)->RequirePrimitive<std::string>();
                    PromptValue pv;
                    pv.mutable_string()->set_text(question);
                    this->SaveMemory(pv, generation);
                } else {
                    auto question = mapping_data.at(options.prompt_variable_key)->RequireMessage<PromptValue>();
                    this->SaveMemory(question, generation);
                }
                return context;
            });
        }

    };

    using ChatMemoryPtr = std::shared_ptr<BaseChatMemory>;
}

#endif //BASECHATMEMORY_HPP
