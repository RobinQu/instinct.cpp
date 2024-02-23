//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include "tools/HttpRestClient.hpp"
#include <nlohmann/json.hpp>

#include "CoreTypes.hpp"
#include "model/BaseLLM.hpp"
#include "model/LLMResult.hpp"
#include "ModelGlobals.hpp"
#include "commons/OllamaCommons.hpp"

LC_LLM_NS {
    class OllamaLLM final : public langchain::core::BaseLLM {
        langchain::core::HttpRestClient http_client_;

    public:
        OllamaLLM();

        explicit OllamaLLM(langchain::core::Endpoint endpoint);

    protected:
        langchain::core::LLMResultPtr Generate(const std::vector<std::string>& prompts,
                                               const std::vector<std::string>& stop_words,
                                               const langchain::core::OptionDict& options) override;
    };

    OllamaLLM::OllamaLLM(): langchain::core::BaseLLM(), http_client_(OLLAMA_ENDPOINT) {
    }

    OllamaLLM::OllamaLLM(langchain::core::Endpoint endpoint): BaseLLM(), http_client_(std::move(endpoint)) {
    }

    langchain::core::LLMResultPtr OllamaLLM::Generate(const std::vector<std::string>& prompts,
                                                      const std::vector<std::string>& stop_words,
                                                      const core::OptionDict& options) {
        auto result = std::make_shared<core::LLMResult>();
        for (const auto& prompt: prompts) {
            // TODO: concurrent execution for multi prompts request
            OllamaGenerateRequest request;
            request.prompt = prompt;
            auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(
                OLLAMA_GENERATE_PATH, request);
            core::OptionDict option_dict = ollama_response;
            auto generations = {
                core::Generation{ollama_response.response, option_dict, "LLMGeneration"}
            };
            result->generations.emplace_back(generations);
        }
        return result;
    }
} // model
// langchian

#endif //OLLAMA_H
