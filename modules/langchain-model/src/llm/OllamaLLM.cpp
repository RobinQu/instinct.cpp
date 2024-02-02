//
// Created by RobinQu on 2024/1/15.
//

#include "OllamaLLM.h"

#include "tools/HttpRequest.h"
#include <fmt/format.h>

namespace langchian::model {

    constexpr std::string OLLAMA_GENERATE_PATH = "/api/generate";

    constexpr langchain::core::Endpoint OLLAMA_DEFAULT_ENDPOINT = {"localhost", 11434};

    OllamaLLM::OllamaLLM(): endpoint(OLLAMA_DEFAULT_ENDPOINT) {
    }

    OllamaLLM::OllamaLLM(const langchain::core::Endpoint& endpoint): endpoint(endpoint) {
    }

    langchain::core::LLMResultPtr OllamaLLM::Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) {
        langchain::core::HttpRequest call {fmt::format("POST {}:{}{}", endpoint.host, endpoint.port, OLLAMA_GENERATE_PATH)};


    }
} // model
// langchian