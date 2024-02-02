//
// Created by RobinQu on 2024/1/15.
//

#include "OllamaLLM.h"

#include "tools/HttpRequest.h"
#include <fmt/format.h>

#include <utility>

namespace langchian::model {

    const static std::string OLLAMA_GENERATE_PATH = "http://localhost:11434/api/generate";

    OllamaLLM::OllamaLLM(): endpoint_(OLLAMA_GENERATE_PATH), http_client_() {
    }

    OllamaLLM::OllamaLLM(std::string  endpoint): endpoint_(std::move(endpoint)), http_client_() {
    }

    langchain::core::LLMResultPtr OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) {
        OllamaGenerateRequest request;
        request.prompt = prompts[0];
        auto ollama_response = http_client_.PostObject<OllamaGenerateRequest, OllamaGenerateResponse>(endpoint_, request);
        auto result = std::make_shared<langchain::core::LLMResult>();
        langchain::core::OptionDict option_dict = ollama_response;
        result->generations.push_back({
            std::make_shared<langchain::core::LLMGeneration>(
                    ollama_response.response,
                    option_dict
                    )});
        return result;
    }
} // model
// langchian