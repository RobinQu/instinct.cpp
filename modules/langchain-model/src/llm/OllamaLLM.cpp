//
// Created by RobinQu on 2024/1/15.
//

#include "OllamaLLM.h"
#include "ModelGlobals.h"
#include "commons/OllamaCommons.h"
#include "model/BaseLLM.h"

namespace LC_MODEL_NS {

    OllamaLLM::OllamaLLM(): langchain::core::BaseLLM(), http_client_(OLLAMA_ENDPOINT) {
    }

    OllamaLLM::OllamaLLM(langchain::core::Endpoint  endpoint): BaseLLM(), http_client_(std::move(endpoint)) {
    }

    langchain::core::LLMResultPtr OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const core::OptionDict& options) {
        auto result = std::make_shared<core::LLMResult>();
        for(const auto& prompt: prompts) {
            // TODO: concurrent execution for multi prompts request
            OllamaGenerateRequest request;
            request.prompt = prompt;
            auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(OLLAMA_GENERATE_PATH, request);
            core::OptionDict option_dict = ollama_response;
            auto generations = {
                {ollama_response.response, option_dict, "LLMGeneration"}
            };
            result->generations.push_back(generations);
        }
        return result;
    }
}
