//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "BaseLanguageModel.h"


namespace langchain::core {
    class BaseLLM: public BaseLanguageModel {
    protected:
        virtual LLMResultPtr Generate(
            const std::vector<std::string>& prompts,
            const std::vector<std::string>& stop_words,
            const OptionDict& options
            ) = 0;
    public:
        BaseLLM() = default;
        ~BaseLLM() override = default;
        LLMResultPtr GeneratePrompt(const std::vector<PromptValuePtr>& prompts,
            const std::vector<std::string>& stop_words, const OptionDict& options) override;
        std::string Predict(const std::string& text, const std::vector<std::string>& stop_words,
            const OptionDict& options) override;
        BaseMessagePtr PredictMessage(const std::vector<BaseMessagePtr>& messages,
            const std::vector<std::string>& stop_words, const OptionDict& options) override;

        std::vector<long> GetTokenIds(const std::string& text) override;

        long GetTokenCount(const std::string& text) override;

        long GetTokenCount(const std::vector<BaseMessagePtr>& messages) override;
    };

}



#endif //BASELLM_H
