//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "BaseLanguageModel.hpp"
#include "CoreGlobals.hpp"
#include "LLMResult.hpp"


LC_CORE_NS {


    // static auto conv_message_to_string = overloaded {
    //     [](const auto& m) {return m.ToString();}
    // };
    //
    // static auto conv_language_model_input_to_string = overloaded{
    //     [](const std::string& v) { return v; },
    //     [](const StringPromptValue& v) { return v.ToString(); },
    //     [](const ChatPromptValue& v) { return v.ToString(); },
    //     [](const MessageVariants& v) {
    //         const auto string_view = v | std::views::transform([](const MessageVariant& m) {
    //             return std::visit(conv_message_to_string, m);
    //         });
    //         return langchian::core::StringUtils::JoinWith(string_view, "\n");
    //     }
    // };

    static auto conv_prompt_value_to_string = overloaded{
        [](const auto& m) {return m.ToString();}
    };

    template<
        typename Configuration,
        typename RuntimeOptions,
        typename Input=LanguageModelInput,
        typename Output=std::string
    >
    class BaseLLM: public BaseLanguageModel<Configuration, RuntimeOptions, Input, Output> {
    protected:
        virtual LLMResult Generate(
            const std::vector<std::string>& prompts,
            const RuntimeOptions& runtime_options
            ) = 0;

        virtual ResultIterator<Generation>* StreamGenerate(const std::string& prompt, const RuntimeOptions& runtime_options ) = 0;
    public:
        // BaseLLM() = default;

        std::string Invoke(
            const LanguageModelInput& input, const RuntimeOptions& options) override {
            const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
        if(const auto result = GeneratePrompts(std::vector{prompt_value}, options); !result.generations.empty()) {
            return std::visit([](auto&& v){return v.text;}, result.generations[0][0]);
        }
        throw LangchainException("Empty response");
        }


        std::string Invoke(
            const LanguageModelInput& input) override {
            return Invoke(input, {});
        }

        /**
         * \brief `Batch` is overriden for LLMs, so that user can reach the real batch API underneath
         * \param input a sequence of prompt input
         * \param options
         * \return
         */
        ResultIterator<std::string>* Batch(
            const std::vector<LanguageModelInput>& input,
            const RuntimeOptions& options) override {
            const auto prompt_value_view = input | std::views::transform([](const auto& v) {
            return std::visit(conv_language_model_input_to_prompt_value, v);
        });

            if(const auto result = GeneratePrompts({prompt_value_view.begin(), prompt_value_view.end()}, options); !result.generations.empty() && !result.generations[0].empty()) {
                auto generation_view = result.generations | std::views::transform([](const std::vector<GenerationVariant>& g) -> std::string {
                    return std::visit([](const auto& v)-> std::string {return v.text;}, g[0]);
                });
                return create_from_range(generation_view);
            }
            throw LangchainException("Empty response");
        }


        ResultIterator<std::string>* Batch(
            const std::vector<LanguageModelInput>& input) override {
            return Batch(input, {});
        }

        ResultIterator<std::string>* Stream(const Input& input, const RuntimeOptions& options) override {
            const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
            const std::string prompt_string = std::visit(conv_prompt_value_to_string, prompt_value);
            ResultIterator<Generation>* geneartion_iter = StreamGenerate(prompt_string, options);
            return create_transform([](const Generation& generation) {
                return generation.text;
            }, geneartion_iter);
        }

        ResultIterator<std::string>* Stream(const Input& input) override {
            return Stream(input, {});
        }

        /**
         * \brief Prompt with model with a batch. for those with batch API, this method will direct these prompts to as a single batch.
         * \param prompts sequence of prompt as a batch
         * \param runtime_options
         * \return
         */
        LLMResult GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                  const RuntimeOptions& runtime_options) override {
            const auto string_view = prompts | std::views::transform([](const PromptValueVairant& pvv) {
            return std::visit(conv_prompt_value_to_string, pvv);
        });
            return Generate({string_view.begin(), string_view.end()}, runtime_options);
        }

    };






}



#endif //BASELLM_H
