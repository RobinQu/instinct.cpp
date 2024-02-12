//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "BaseLanguageModel.h"
#include "ChatResult.h"


namespace langchain::core {

class BaseChatModel: public BaseLanguageModel {
protected:
    virtual ChatResultPtr Generate(
            const std::vector<std::vector<BaseMessagePtr>>& messages,
            const std::vector<std::string>& stop_words,
            const OptionDict& options
            ) = 0;

public:
    LLMResultPtr GeneratePrompt(const std::vector<PromptValuePtr>& prompts, const std::vector<std::string>& stop_words,
        const OptionDict& options) override;

    std::string Predict(const std::string& text, const std::vector<std::string>& stop_words,
        const OptionDict& options) override;

    BaseMessagePtr PredictMessage(const std::vector<BaseMessagePtr>& messages,
        const std::vector<std::string>& stop_words, const OptionDict& options) override;

    std::vector<long> GetTokenIds(const std::string& text) override;

    long GetTokenCount(const std::string& text) override;

    long GetTokenCount(const std::vector<BaseMessagePtr>& messages) override;
};

} // core
// langchain

#endif //BASECHATMODEL_H
