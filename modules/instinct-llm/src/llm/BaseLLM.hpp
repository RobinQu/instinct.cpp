//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "model/ILanguageModel.hpp"
#include "LLMGlobals.hpp"


namespace
INSTINCT_LLM_NS {
    class BaseLLM : public ILanguageModel<std::string> {
        virtual BatchedLangaugeModelResult Generate(const std::vector<std::string>& prompts) = 0;

        virtual ResultIteratorPtr<LangaugeModelResult> StreamGenerate(const std::string& prompt) = 0;

    public:
        std::string Invoke(
            const PromptValueVariant& input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            if (const auto result = Generate({string_prompt}); !result.generations().empty()) {
                return details::conv_languange_result_to_string(result.generations(0));
            }
            throw InstinctException("Empty response");
        }

        ResultIteratorPtr<std::string> Batch(
            const std::vector<PromptValueVariant>& input) override {
            auto string_view = input | std::views::transform(details::conv_prompt_value_variant_to_string);
            auto batched_result = Generate({string_view.begin(), string_view.end()});
            if (batched_result.generations_size() > 0) {
                auto result_string_view = batched_result.generations() | std::views::transform(
                                              details::conv_languange_result_to_string);
                return create_result_itr_from_range(result_string_view);
            }
            throw InstinctException("Empty response");
        }

        ResultIteratorPtr<std::string> Stream(const PromptValueVariant& input) override {
            const auto string_prompt = details::conv_prompt_value_variant_to_string(input);
            return create_result_itr_with_transform(details::conv_languange_result_to_string,
                                                    StreamGenerate(string_prompt));
        }
    };

    using LLMPtr = std::shared_ptr<BaseLLM>;
}


#endif //BASELLM_H
