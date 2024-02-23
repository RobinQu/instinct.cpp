//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "BaseLanguageModel.hpp"
#include "ChatResult.hpp"
#include "LLMResult.hpp"


LC_CORE_NS {
    static auto conv_prompt_value_to_message = overloaded {
        [](const StringPromptValue& v) -> MessageVariants {return v.ToMessages();},
        [](const ChatPromptValue& v) -> MessageVariants {return v.ToMessages();}
    };


    class BaseChatModel : public BaseLanguageModel<MessageVariant> {
    protected:
        virtual LLMResult Generate(
            const std::vector<MessageVariants>& messages,
            const LLMRuntimeOptions& runtime_options
        ) = 0;

    public:
        MessageVariant Invoke(
            const LanguageModelInput& input, const LLMRuntimeOptions& options) override;

        LLMResult GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                  const LLMRuntimeOptions& runtime_options) override;

    };

    inline MessageVariant BaseChatModel::Invoke(const LanguageModelInput& input, const LLMRuntimeOptions& options) {
        const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
        if(const auto result = GeneratePrompts(std::vector{prompt_value}, options); !result.generations.empty()) {
            return std::visit([]<typename X>(X &&arg) {
                using T = std::decay_t<decltype(arg)>;
               if constexpr(std::is_same_v<T, ChatGeneration>) {
                   return MessageVariant{arg.message};
               } else {
                   static_assert(always_false_v<T>, "generation should be kind of ChatGeneration.");
               }
            }, result.generations[0][0]);
        }
        throw LangchainException("Empty response");
    }

    inline LLMResult BaseChatModel::GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
        const LLMRuntimeOptions& runtime_options) {
        const auto messages_view = prompts | std::views::transform([](const PromptValueVairant& pvv) {
            return std::visit(conv_prompt_value_to_message, pvv);
        });
        return Generate({messages_view.begin(), messages_view.end()}, runtime_options);
    }

} // core
// langchain

#endif //BASECHATMODEL_H
