//
// Created by RobinQu on 2024/1/13.
//

#include "BaseChatModel.h"
#include <ranges>

#include "message/HumanMessage.h"

namespace LC_CORE_NS {
    LLMResultPtr BaseChatModel::GeneratePrompt(const std::vector<PromptValuePtr>& prompts,
        const std::vector<std::string>& stop_words, const OptionDict& options) {
        auto messages_range = prompts | std::views::transform([](const auto& value) {
                    return value->ToMessages();
                });
        ChatResultPtr chat_result = Generate({messages_range.begin(), messages_range.end()}, stop_words, options);
        LLMResultPtr llm_result = std::make_shared<LLMResult> {
            chat_result->generations,
            chat_result->llm_output
        };
        return llm_result;
    }

    std::string BaseChatModel::Predict(const std::string& text, const std::vector<std::string>& stop_words,
        const OptionDict& options) {
        auto messages = {{std::make_shared<HumanMessage>(text)}};
        auto result = Generate(messages, stop_words, options);
        return result->generations[0].text;
    }

    BaseMessagePtr BaseChatModel::PredictMessage(const std::vector<BaseMessagePtr>& messages,
        const std::vector<std::string>& stop_words, const OptionDict& options) {
        auto result = Generate({messages}, stop_words, options);
        auto message = result->generations[0].message;
        return std::make_shared<ChatMessage>(message);
    }

    std::vector<long> BaseChatModel::GetTokenIds(const std::string& text) {
    }

    long BaseChatModel::GetTokenCount(const std::string& text) {
    }

    long BaseChatModel::GetTokenCount(const std::vector<BaseMessagePtr>& messages) {
    }
} // core
// langchain