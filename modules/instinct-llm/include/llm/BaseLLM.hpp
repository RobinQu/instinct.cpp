//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "model/ILanguageModel.hpp"
#include "LLMGlobals.hpp"
#include "model/ModelCallbackMixins.hpp"
#include "functional/StepFunctions.hpp"


namespace INSTINCT_LLM_NS {
    class BaseLLM : public ILanguageModel<std::string>, public BaseStepFunction {
        virtual BatchedLangaugeModelResult Generate(const std::vector<std::string>& prompts) = 0;

        virtual AsyncIterator<LangaugeModelResult> StreamGenerate(const std::string& prompt) = 0;

    public:
        std::string Invoke(
            const PromptValueVariant& input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            if (const auto result = Generate({string_prompt}); !result.generations().empty()) {
                const auto answer = details::conv_languange_result_to_string(result.generations(0));
                return answer;
            }
            throw InstinctException("Empty response");
        }

        AsyncIterator<std::string> Batch(
            const std::vector<PromptValueVariant>& input) override {
            auto string_view = input | std::views::transform(details::conv_prompt_value_variant_to_string);
            auto batched_result = Generate({string_view.begin(), string_view.end()});
            if (batched_result.generations_size() > 0) {
                auto result_string_view = batched_result.generations() | std::views::transform(
                                              details::conv_languange_result_to_string);
                return rpp::source::from_iterable(result_string_view);
            }
            return CreateAsyncIteratorWithError<std::string>("Empty response");
        }

        AsyncIterator<std::string> Stream(const PromptValueVariant& input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            return StreamGenerate(string_prompt)
                | rpp::operators::map(details::conv_languange_result_to_string);
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto prompt = input->RequirePrimitive<std::string>(DEFAULT_PROMPT_INPUT_KEY);
            auto answer = Invoke(prompt);
            JSONContextPtr result = input;
            result->PutPrimitive(DEFAULT_ANSWER_OUTPUT_KEY, answer);
            return result;
        }

        AsyncIterator<instinct::core::JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override {
            auto prompts_view = input | std::views::transform([&](const auto& ctx) {
                auto prompt = ctx->template RequirePrimitive<std::string>(DEFAULT_PROMPT_INPUT_KEY);
                return PromptValueVariant  {prompt};
            });



            return Batch(std::vector<PromptValueVariant> {prompts_view.begin(), prompts_view.end()})
                | rpp::operators::map([](const std::string& answer) {
                    auto ctx = CreateJSONContext();
                    ctx->PutPrimitive(DEFAULT_ANSWER_OUTPUT_KEY, answer);
                    return ctx;
                });
            ;
        }

        AsyncIterator<instinct::core::JSONContextPtr> Stream(const JSONContextPtr &input) override {
            auto prompt = input->RequirePrimitive<std::string>(DEFAULT_PROMPT_INPUT_KEY);
            return Stream(prompt)
                | rpp::operators::map([&](const auto& answer) {
                    auto ctx = CreateJSONContext();
                    ctx->PutPrimitive(DEFAULT_ANSWER_OUTPUT_KEY, answer);
                    return ctx;
                });
        }

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return {DEFAULT_PROMPT_INPUT_KEY};
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return {DEFAULT_ANSWER_OUTPUT_KEY};
        }
    };

    using LLMPtr = std::shared_ptr<BaseLLM>;
}


#endif //BASELLM_H
