//
// Created by RobinQu on 2024/1/15.
//

#include "BaseLLM.h"
#include "CoreFunctions.h"
#include "message/AIMessage.h"

namespace langchain::core {

    LLMResultPtr BaseLLM::GeneratePrompt(const std::vector<PromptValuePtr>& prompts,
                                         const std::vector<std::string>& stop_words, const OptionDict& options) {
        std::vector<std::string> prompt_strings(prompts.size());
        for(const PromptValuePtr& prompt_value: prompts) {
            prompt_strings.push_back(prompt_value->ToString());
        }
        return Generate(prompt_strings, stop_words, options);
    }

    std::string BaseLLM::Predict(const std::string& text, const std::vector<std::string>& stop_words,
        const OptionDict& options) {
        const std::vector<std::string> prompt_strings {text};
        if (const LLMResultPtr result = Generate(prompt_strings, stop_words, options); result && !result->generations.empty() && !result->generations[0].empty()) {
            return result->generations[0][0]->text;
        }
        throw LangchainException("Empty response");
    }

    BaseMessagePtr BaseLLM::PredictMessage(const std::vector<BaseMessagePtr>& messages,
        const std::vector<std::string>& stop_words, const OptionDict& options) {
        std::string text = GetBufferString(messages);
        if(const LLMResultPtr result = Generate({text}, stop_words, options); result && !result->generations.empty() && !result->generations[0].empty()) {
            return std::make_shared<AIMessage>(result->generations[0][0]->text);
        }
        throw LangchainException("Empty response");
    }

    std::vector<long> BaseLLM::GetTokenIds(const std::string& text) {
    }

    long BaseLLM::GetTokenCount(const std::string& text) {
    }

    long BaseLLM::GetTokenCount(const std::vector<BaseMessagePtr>& messages) {

    }
}

