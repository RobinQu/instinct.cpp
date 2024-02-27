//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "BaseLanguageModel.hpp"
#include "ChatResult.hpp"
#include "LLMResult.hpp"


namespace INSTINCT_CORE_NS {
    static auto conv_prompt_value_to_message = overloaded{
        [](const StringPromptValue& v) -> MessageVariants { return v.ToMessages(); },
        [](const ChatPromptValue& v) -> MessageVariants { return v.ToMessages(); }
    };

    static auto conv_message_to_string = overloaded {
        [](const AIMessage& v) {return v.ToString();},
        [](const HumanMessage& v) {return v.ToString();},
        [](const FunctionMessage& v) {return v.ToString();},
        [](const ChatMessage& v) {return v.ToString();},
        [](const SystemMessage& v) {return v.ToString();},
    };

    template<
        typename Configuration,
        typename RuntimeOptions,
        typename Input=LanguageModelInput,
        typename Output=MessageVariant>
    class BaseChatModel : public BaseLanguageModel<Configuration, RuntimeOptions, Input, Output> {
    protected:
        virtual ChatResult Generate(
            const std::vector<MessageVariants>& messages,
            const RuntimeOptions& runtime_options
        ) = 0;

        virtual ResultIterator<ChatGeneration>* StreamGenerate(const MessageVariants& messages, const RuntimeOptions& runtime_options) = 0;

    public:
        MessageVariant Invoke(
            const LanguageModelInput& input, const RuntimeOptions& options) override {
            const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
            if (const auto result = GeneratePrompts(std::vector{prompt_value}, options);
                !result.generations.empty() && !result.generations[0].empty()) {
                const auto generation = result.generations[0][0];
                if (std::holds_alternative<ChatGeneration>(generation)) {
                    auto chat_gen = std::get<ChatGeneration>(generation);
                    return chat_gen.message;
                }
                throw LangchainException("unexpected branch here");
            }
            throw LangchainException("Empty response");
        }

        Output Invoke(const Input& input) override {
            return Invoke(input, {});
        }

        /**
         * \brief Batching is disabled on chat models. This function throws unconditionally.
         * \param input
         * \param options
         * \return a
         */
        ResultIterator<MessageVariant>* Batch(
            const std::vector<LanguageModelInput>& input,
            const RuntimeOptions& options) override {
            const auto prompt_view = input | std::views::transform([](const auto& e) {
                return std::visit(conv_language_model_input_to_prompt_value, e);
            });
            const auto llm_result = GeneratePrompts({prompt_view.begin(), prompt_view.end()}, options);

            MessageVariants mvs;
            for (const auto& mv: llm_result.generations) {
                if (!mv.empty()) {
                    if (std::holds_alternative<ChatGeneration>(mv[0])) {
                        const auto generation = std::get<ChatGeneration>(mv[0]);
                        mvs.push_back(generation.message);
                    }
                }
            }
            if (mvs.empty()) {
                throw LangchainException("Empty response");
            }
            return create_from_range(mvs);
        }

        ResultIterator<Output>* Batch(const std::vector<Input>& input) override {
            return Batch(input, {});
        }


        ResultIterator<MessageVariant>* Stream(
            const LanguageModelInput& input,
            const RuntimeOptions& options) override {
            const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
            const auto message = std::visit(conv_prompt_value_to_message, prompt_value);
            ResultIterator<ChatGeneration>* generation_result = StreamGenerate(message, options);
            return create_transform([](auto&& chat_gen) -> MessageVariant {
                return chat_gen.message;
            }, generation_result);
        }

        ResultIterator<MessageVariant>* Stream(
            const LanguageModelInput& input) override {
            return Stream(input, {});
        }

        LLMResult GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                  const RuntimeOptions& runtime_options) override {
            const auto messages_view = prompts | std::views::transform([](const PromptValueVairant& pvv) {
                return std::visit(conv_prompt_value_to_message, pvv);
            });
            const ChatResult chat_result = Generate({messages_view.begin(), messages_view.end()}, runtime_options);
            LLMResult result;
            for(const auto& chat_gen: chat_result.generations) {
                result.generations.push_back({GenerationVariant{chat_gen}});
            }
            result.llm_output  = chat_result.llm_output;
            return result;
        };
    };
}

// core
// langchain

#endif //BASECHATMODEL_H
