//
// Created by RobinQu on 2024/2/12.
//

#ifndef OLLAMACHAT_H
#define OLLAMACHAT_H
#include "model/BaseChatModel.hpp"
#include "tools/HttpRestClient.hpp"
#include "ModelGlobals.hpp"
#include "commons/OllamaCommons.hpp"

LC_LLM_NS {
    static auto conv_message_variant_to_ollama_message = core::overloaded {
      [](const core::AIMessage& v) {return OllamaGenerateMessage{"assistant", v.GetContent()};},
        [](const core::SystemMessage& v) {return OllamaGenerateMessage{"system", v.GetContent()};},
        [](const core::HumanMessage& v) {return OllamaGenerateMessage{"user", v.GetContent()};},
        [](const core::FunctionMessage& v) {return OllamaGenerateMessage{"assistant", v.GetContent()};},
        [](const core::ChatMessage& v) {return OllamaGenerateMessage{v.GetRole(), v.GetContent()};},
    };


    class OllamaChat : public core::BaseChatModel<OllamaConfiguration, OllamaRuntimeOptions> {
        core::HttpRestClient client_;

    public:
        OllamaChat(): client_(OLLAMA_ENDPOINT) {
        };

        explicit OllamaChat(core::Endpoint endpoint): client_(std::move(endpoint)) {
        }

        std::vector<core::TokenId> GetTokenIds(const std::string& text) override;

        core::TokenSize GetTokenCount(const std::string& text) override;

        core::TokenSize GetTokenCount(const core::MessageVariants& messages) override;



    protected:
        core::ResultIterator<core::ChatGeneration>* StreamGenerate(const core::MessageVariants& messages,
            const OllamaRuntimeOptions& runtime_options) override;

        core::ChatResult Generate(const std::vector<core::MessageVariants>& messages,
            const OllamaRuntimeOptions& runtime_options) override;
    };


    inline std::vector<core::TokenId> OllamaChat::GetTokenIds(const std::string& text) {
        return {};
    }

    inline core::TokenSize OllamaChat::GetTokenCount(const std::string& text) {
        return 0;
    }

    inline core::TokenSize OllamaChat::GetTokenCount(const core::MessageVariants& messages) {
        return 0;
    }

    inline core::ResultIterator<core::ChatGeneration>* OllamaChat::StreamGenerate(const core::MessageVariants& messages,
        const OllamaRuntimeOptions& runtime_options) {
        auto ollama_messages = messages | std::views::transform([](const core::MessageVariant& m) {
                return std::visit(conv_message_variant_to_ollama_message, m);
            });

        OllamaChatRequest request = {
            runtime_options.model_name,
            {ollama_messages.begin(), ollama_messages.end()},
            true
        };

        auto* resposne_result = client_.StreamChunk<OllamaChatRequest, OllamaGenerateResponse>(OLLAMA_CHAT_PATH, request);
        return create_transform([](const OllamaGenerateResponse& response) {
            core::OptionDict option_dict = response;
            return core::ChatGeneration {response.message.content,
                option_dict,
                core::ChatMessage {response.message.content, response.message.role},
                };
        }, resposne_result);
    }

    inline core::ChatResult OllamaChat::Generate(const std::vector<core::MessageVariants>& messages_batch,
        const OllamaRuntimeOptions& runtime_options) {
        core::ChatResult result;
        for (const auto& messages: messages_batch) {
            auto ollama_messages = messages | std::views::transform([](const core::MessageVariant& m) {
                return std::visit(conv_message_variant_to_ollama_message, m);
            });
            OllamaChatRequest request = {
                runtime_options.model_name,
                {ollama_messages.begin(), ollama_messages.end()},

                false
            };
            OllamaGenerateResponse response = client_.PostObject<OllamaChatRequest, OllamaGenerateResponse>(
                OLLAMA_CHAT_PATH, request);
            core::OptionDict option_dict = response;

            std::vector<core::ChatGeneration> geneartions;
            result.generations.emplace_back(core::ChatGeneration {
                response.message.content,
                std::move(option_dict),
                core::ChatMessage {response.message.content, response.message.role},
            });

        }
        return result;
    }

} // core
// langchain

#endif //OLLAMACHAT_H
