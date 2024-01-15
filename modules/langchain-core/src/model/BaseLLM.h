//
// Created by RobinQu on 2024/1/15.
//

#ifndef BASELLM_H
#define BASELLM_H
#include "BaseLanguageModel.h"


namespace langchain::core {
    class BaseLLM: langchain::core::BaseLanguageModel {
    protected:
        virtual langchain::core::LLMResultPtr Generate(
            std::vector<std::string>& prompts,
            std::vector<std::string>& stop_words,
            const OptionDict& options
            ) = 0;
    public:
        BaseLLM() = default;
        virtual ~BaseLLM() = 0;
        langchain::core::LLMResultPtr GeneratePrompt(std::vector<langchain::core::PromptValuePtr>& prompts,
            std::vector<std::string>& stop_words, langchain::core::OptionDict& options) override;
        std::string Predict(std::string& text, std::vector<std::string>& stop_words,
            langchain::core::OptionDict& options) override;
        langchain::core::BaseMessagePtr PredictMessage(std::vector<langchain::core::BaseMessagePtr>& messages,
            std::vector<std::string>& stop_words, langchain::core::OptionDict& options) override;
    };

}



#endif //BASELLM_H
