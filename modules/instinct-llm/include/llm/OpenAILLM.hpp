//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAILLM_HPP
#define OPENAILLM_HPP

#include <utility>

#include "BaseLLM.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "commons/OllamaCommons.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * OpenAI text completion is depreciated, and it's recommended to make use of ChatCompletion instead. OpenAILLM is just a wrapper around `OpenAIChat` for conventions.
     */
    class OpenAILLM final: public BaseLLM {
        OpenAIChat chat_;

    public:
        explicit OpenAILLM(OpenAIConfiguration configuration)
            : BaseLLM(configuration.base_options), chat_(std::move(configuration)) {
        }

        void BindToolSchemas(const std::vector<FunctionTool> &function_tool_schema) override {
            chat_.BindToolSchemas(function_tool_schema);
        }

    private:
        BatchedLangaugeModelResult Generate(const std::vector<std::string>& prompts) override {
            auto message_matrix = prompts | std::views::transform([](const auto& prompt) {
                MessageList message_list;
                auto* message = message_list.add_messages();
                // TODO role name normalization
                message->set_role("user");
                message->set_content(prompt);
                return message_list;
            });

            return chat_.Generate({message_matrix.begin(), message_matrix.end()});
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const std::string& prompt) override {
            MessageList message_list;
            auto* msg = message_list.add_messages();
            msg->set_content(prompt);
            msg->set_role("user");
            return chat_.StreamGenerate(message_list);
        }
    };

    static LLMPtr CreateOpenAILLM(const OpenAIConfiguration& configuration) {
        return std::make_shared<OpenAILLM>(configuration);
    }
}

#endif //OPENAILLM_HPP
