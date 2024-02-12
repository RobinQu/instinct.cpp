//
// Created by RobinQu on 2024/1/15.
//

#include "OllamaLLM.h"

#include "tools/HttpRequest.h"
#include <fmt/format.h>

#include <utility>

namespace langchian::model {

    static const langchain::core::Endpoint OLLAMA_ENDPOINT {"localhost", 11434};

    static const std::string OLLAMA_GENERATE_PATH = "/api/generate";

    OllamaLLM::OllamaLLM():BaseLLM(), http_client_(OLLAMA_ENDPOINT) {
    }

    OllamaLLM::OllamaLLM(langchain::core::Endpoint  endpoint): BaseLLM(), http_client_(std::move(endpoint)) {
    }

    langchain::core::LLMResultPtr OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) {
        OllamaGenerateRequest request;
        request.prompt = prompts[0];
        auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(OLLAMA_GENERATE_PATH, request);
        auto result = std::make_shared<langchain::core::LLMResult>();
        langchain::core::OptionDict option_dict = ollama_response;
        result->generations.push_back({
            {ollama_response.response, option_dict}
        });
        return result;
    }
} // model
// langchian