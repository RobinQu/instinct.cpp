//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "BaseLanguageModel.hpp"
#include "CoreGlobals.hpp"
#include "LLMResult.h"


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

    class BaseLLM: public BaseLanguageModel<std::string>{
    protected:
        virtual LLMResult Generate(
            const std::vector<std::string>& prompts,
            const LLMRuntimeOptions& runtime_options
            );
    public:
        // BaseLLM() = default;


        std::string Invoke(
            const LanguageModelInput& input, const LLMRuntimeOptions& options) override;


        /**
         * \brief `Batch` is overriden for LLMs, so that user can reach the real batch API underneath
         * \param input a sequence of prompt input
         * \param options
         * \return
         */
        std::vector<std::string> Batch(
            const std::vector<LanguageModelInput>& input,
            const LLMRuntimeOptions& options) override;

        /**
         * \brief Prompt with model with a batch. for those with batch API, this method will direct these prompts to as a single batch.
         * \param prompts sequence of prompt as a batch
         * \param runtime_options
         * \return
         */
        LLMResult GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                  const LLMRuntimeOptions& runtime_options) override;


    };

    inline std::string BaseLLM::Invoke(const LanguageModelInput& input, const LLMRuntimeOptions& options) {
        const auto prompt_value = std::visit(conv_language_model_input_to_prompt_value, input);
        if(const auto result = GeneratePrompts(std::vector{prompt_value}, options); !result.generations.empty()) {
            return std::visit([](auto&& v){return v.text;}, result.generations[0][0]);
        }
        throw LangchainException("Empty response");
    }

    inline LLMResult BaseLLM::GeneratePrompts(const std::vector<PromptValueVairant>& prompts,
                                              const LLMRuntimeOptions& runtime_options) override {
        const auto string_view = prompts | std::views::transform([](const PromptValueVairant& pvv) {
            return std::visit(conv_prompt_value_to_string, pvv);
        });
        return Generate({string_view.begin(), string_view.end()}, runtime_options);
    }


    inline std::vector<std::string> BaseLLM::Batch(const std::vector<LanguageModelInput>& input,
        const LLMRuntimeOptions& options) {
        const auto prompt_value_view = input | std::views::transform([](const auto& v) {
            return std::visit(conv_language_model_input_to_prompt_value, v);
        });

        if(const auto result = GeneratePrompts({prompt_value_view.begin(), prompt_value_view.end()}, options); !result.generations.empty()) {
            auto generation_view = result.generations | std::views::transform([](const std::vector<Generation>& g) {
                return g[0].text;
            });
            return {generation_view.begin(), generation_view.end()};
        }
        throw LangchainException("Empty response");
    }
}



#endif //BASELLM_H
