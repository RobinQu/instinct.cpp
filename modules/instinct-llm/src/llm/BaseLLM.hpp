//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "model/ILanguageModel.hpp"
#include "LLMGlobals.hpp"


namespace INSTINCT_LLM_NS {

    class BaseLLM: public ILanguageModel {
    private:
        virtual BatchedLangaugeModelResult Generate(const std::vector<std::string>& prompts) = 0;

        virtual ResultIteratorPtr<LangaugeModelResult> StreamGenerate(const std::string& prompt) = 0;
    public:
        // BaseLLM() = default;

        LangaugeModelResult Invoke(
            const PromptValue& input) override {
            const auto string_prompt = convert_prompt_value_to_string(input);
        if(const auto result = Generate({string_prompt}); !result.generations.empty()) {
            return result.generations(0);
        }
        throw InstinctException("Empty response");
        }

        /**
         * \brief `Batch` is overriden for LLMs, so that user can reach the real batch API underneath
         * \param input a sequence of prompt input
         * \param options
         * \return
         */
        ResultIteratorPtr<LangaugeModelResult> Batch(
            const std::vector<PromptValue>& input) override {
            auto string_view = input | std::views::transform(convert_prompt_value_to_string);
            auto batched_result = Generate({string_view.begin(), string_view.end()});
            if (batched_result.generations_size()>0) {
                return create_result_itr_from_range(batched_result.generations());
            }
            throw InstinctException("Empty response");
        }

        ResultIteratorPtr<LangaugeModelResult> Stream(const PromptValue& input) override {
            const auto string_prompt = convert_prompt_value_to_string(input);
            return StreamGenerate(string_prompt);
        }

    };

    using LLMPtr = std::shared_ptr<BaseLLM>;


}



#endif //BASELLM_H
