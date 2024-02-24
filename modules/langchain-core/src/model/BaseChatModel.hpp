//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "BaseLanguageModel.hpp"
#include "ChatResult.hpp"
#include "LLMResult.hpp"


LC_CORE_NS {
    static auto conv_prompt_value_to_message = overloaded{
        [](const StringPromptValue& v) -> MessageVariants { return v.ToMessages(); },
        [](const ChatPromptValue& v) -> MessageVariants { return v.ToMessages(); }
    };

    static auto conv_message_to_string = [](const auto& v) {
        return v.ToString();
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

        /**
         * \brief Batching is disabled on chat models. This function throws unconditionally.
         * \param input
         * \param a
         * \return a
         */
        MessageVariants Batch(
            const std::vector<LanguageModelInput>& input,
            const LLMRuntimeOptions& options) override;


        std::vector<MessageVar> Stream(
            const std::variant<StringPromptValue, ChatPromptValue, std::string, std::vector<std::variant<AIMessage,
            HumanMessage, FunctionMessage, SystemMessage, ChatMessage>>>& input,
            const LLMRuntimeOptions& options) override;

        std::vector<std::variant<AIMessage, HumanMessage, FunctionMessage, SystemMessage, ChatMessage>> Stream(
            const std::variant<StringPromptValue, ChatPromptValue, std::string, std::vector<std::variant<AIMessage,
            HumanMessage, FunctionMessage, SystemMessage, ChatMessage>>>& input) override;

        LLMResult GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                  const LLMRuntimeOptions& runtime_options) override;
    };

    inline MessageVariant BaseChatModel::Invoke(const LanguageModelInput& input, const LLMRuntimeOptions& options) {
        const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
        if (const auto result = GeneratePrompts(std::vector{prompt_value}, options);
            !result.generations.empty() && !result.generations[0].empty()) {
            const auto generation = result.generations[0][0];
            if(std::holds_alternative<ChatGeneration>(generation)) {
                auto chat_gen = std::get<ChatGeneration>(generation);
                return chat_gen.message;
            }
            throw LangchainException("unexpected branch here");
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

    inline MessageVariants BaseChatModel::Batch(const std::vector<LanguageModelInput>& input,
        const LLMRuntimeOptions& options) {
        const auto prompt_view = input | std::views::transform([](const auto& e) {
            return std::visit(conv_language_model_input_to_prompt_value, e);
        });
        const auto llm_result = GeneratePrompts({prompt_view.begin(), prompt_view.end()}, options);

        MessageVariants mvs;
        for(const auto& mv: llm_result.generations) {
            if(!mv.empty()) {
                if(std::holds_alternative<ChatGeneration>(mv[0])) {
                    const auto generation = std::get<ChatGeneration>(mv[0]);
                    mvs.emplace_back(generation.message);
                }
            }
        }
        if(mvs.empty()) {
            throw LangchainException("Empty response");
        }
        return mvs;
    }
} // core
// langchain

#endif //BASECHATMODEL_H
