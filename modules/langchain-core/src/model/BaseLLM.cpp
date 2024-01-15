//
// Created by RobinQu on 2024/1/15.
//

#include "BaseLLM.h"

namespace langchain::core {

    LLMResultPtr BaseLLM::GeneratePrompt(std::vector<PromptValuePtr>& prompts,
                                         std::vector<std::string>& stop_words, OptionDict& options) {
        std::vector<std::string> prompt_strings;
        for(const PromptValuePtr& prompt_value: prompts) {
            prompt_strings.push_back(prompt_value->ToString());
        }
        return Generate(prompt_strings, stop_words, options);
    }

    std::string BaseLLM::Predict(std::string& text, std::vector<std::string>& stop_words,
        OptionDict& options) {
        std::vector<std::string> prompt_strings {text};
        if (const LLMResultPtr result = Generate(prompt_strings, stop_words, options); result && !result->generations.empty() && !result->generations[0].empty()) {
            return result->generations[0][0]->text;
        }
        throw LangchainException("Empty response");
    }

    BaseMessagePtr BaseLLM::PredictMessage(std::vector<BaseMessagePtr>& messages,
        std::vector<std::string>& stop_words, OptionDict& options) {

    }

}

