//
// Created by RobinQu on 2024/1/12.
//

#ifndef BASELANGUAGEMODEL_H
#define BASELANGUAGEMODEL_H
#include <future>

#include "LLMResult.h"
#include "../prompt/PromptValue.h"

namespace langchain {
namespace core {

class BaseLanguageModel {

public:
    virtual LLMResultPtr GeneratePrompt(
        std::vector<PromptValuePtr>& prompts,
        std::vector<std::string>& stop_words,
        OptionDict& options
        ) = 0;

    virtual std::string Predict(
        std::string& text,
        std::vector<std::string>& stop_words,
        OptionDict& options
        ) = 0;

    virtual BaseMessagePtr PredictMessage(
        std::vector<BaseMessagePtr>& messages,
        std::vector<std::string>& stop_words,
        OptionDict& options
        ) = 0;

    // virtual std::future<LLMResultPtr> GeneratePromptAsync(
    // std::vector<PromptValuePtr>& prompts,
    // std::vector<std::string>& stop_words,
    // OptionDict& options
    // ) = 0;
    //
    // virtual std::future<std::string> PredictAsync(
    //     std::string& text,
    //     std::vector<std::string>& stop_words,
    //     OptionDict& options
    //     ) = 0;
    //
    // virtual std::future<BaseMessagePtr> PredictMessageAsync(
    // std::vector<BaseMessagePtr>& messages,
    // std::vector<std::string>& stop_words,
    // OptionDict& options
    // ) = 0;

    virtual std::vector<long> GetTokenIds(std::string& text) = 0;

    virtual long GetTokenCount(std::string& text) = 0;

    virtual long GetTokenCount(std::vector<BaseMessagePtr>& messages) = 0;

};

} // core
} // langchian

#endif //BASELANGUAGEMODEL_H
