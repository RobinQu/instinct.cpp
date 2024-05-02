//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H

#include <utility>

#include "model/ILanguageModel.hpp"
#include "LLMGlobals.hpp"
#include "functional/StepFunctions.hpp"


namespace INSTINCT_LLM_NS {
    class BaseLLM;

    using LLMPtr = std::shared_ptr<BaseLLM>;

    class LLMStepFunction : public BaseStepFunction {
        LLMPtr model_;
    public:
        explicit LLMStepFunction(LLMPtr model) : model_(std::move(model)) {};

        JSONContextPtr Invoke(const JSONContextPtr &input) override;

        AsyncIterator<JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override;

        AsyncIterator<JSONContextPtr> Stream(const JSONContextPtr &input) override;

    };


    class BaseLLM
            : public virtual ILanguageModel,
              public virtual IConfigurable<ModelOptions>,
              public BaseRunnable<PromptValueVariant, std::string>,
              public std::enable_shared_from_this<BaseLLM> {
        friend LLMStepFunction;

        ModelOptions options_;

        virtual BatchedLangaugeModelResult Generate(const std::vector<std::string> &prompts) = 0;

        virtual AsyncIterator<LangaugeModelResult> StreamGenerate(const std::string &prompt) = 0;

    public:
        explicit BaseLLM(const ModelOptions &options) : options_(options) {
        }

        void Configure(const ModelOptions &options) override {
            options_ = options;
        }

        std::string Invoke(
                const PromptValueVariant &input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            if (const auto result = Generate({string_prompt}); !result.generations().empty()) {
                const auto answer = details::conv_language_result_to_string(result.generations(0));
                return answer;
            }
            throw InstinctException("Empty response");
        }

        AsyncIterator<std::string> Batch(
                const std::vector<PromptValueVariant> &input) override {
            auto string_view = input | std::views::transform(details::conv_prompt_value_variant_to_string);
            auto batched_result = Generate({string_view.begin(), string_view.end()});
            if (batched_result.generations_size() > 0) {
                auto result_string_view = batched_result.generations() | std::views::transform(
                        details::conv_language_result_to_string);
                return rpp::source::from_iterable(result_string_view);
            }
            return CreateAsyncIteratorWithError<std::string>("Empty response");
        }

        AsyncIterator<std::string> Stream(const PromptValueVariant &input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            return StreamGenerate(string_prompt)
                   | rpp::operators::map(details::conv_language_result_to_string);
        }

        StepFunctionPtr AsModelFunction()  {
            return std::make_shared<LLMStepFunction>(shared_from_this());;
        }

        void BindToolSchemas(const std::vector<FunctionToolSchema> &function_tool_schema) override {
            throw InstinctException("Not implemented");
        }

    };


    inline JSONContextPtr LLMStepFunction::Invoke(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>();
        const auto string_prompt = details::conv_prompt_value_variant_to_string(prompt_value);

        auto batched_result = model_->Generate({string_prompt});
        assert_non_empty_range(batched_result.generations(), "Empty Response");
        assert_non_empty_range(batched_result.generations(0).generations(), "Empty Response in first model output");

        input->ProduceMessage(batched_result.generations(0).generations(0));
        return input;
    }

    inline AsyncIterator<instinct::core::JSONContextPtr> LLMStepFunction::Batch(const std::vector<JSONContextPtr> &input) {
        auto prompts_view = input | std::views::transform([&](const JSONContextPtr &ctx) {
            auto prompt = ctx->template RequirePrimitive<std::string>();
            return details::conv_prompt_value_variant_to_string(prompt);
        });

        auto batched_result = model_->Generate({prompts_view.begin(), prompts_view.end()});
        return rpp::source::from_iterable(batched_result.generations())
               | rpp::operators::zip(rpp::source::from_iterable(input))
               | rpp::operators::map([&](const std::tuple<LangaugeModelResult, JSONContextPtr> &tuple) {
            auto ctx = std::get<1>(tuple);
            LangaugeModelResult answer = std::get<0>(tuple);
            ctx->ProduceMessage(answer.generations(0));
            return ctx;
        });
    }

    inline AsyncIterator<instinct::core::JSONContextPtr> LLMStepFunction::Stream(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>();
        const auto string_prompt = details::conv_prompt_value_variant_to_string(prompt_value);
        return model_->StreamGenerate(string_prompt)
               | rpp::operators::map([&](const LangaugeModelResult &answer) {
            auto ctx = CreateJSONContext();
            ctx->ProduceMessage(answer.generations(0));
            return ctx;
        });
    }



}


#endif //BASELLM_H
