//
// Created by RobinQu on 2024/1/15.
//

#include "Ollama.h"


static const std::string OLLAMA_ENDPOINT = "http://localhost:11434/api/generate";

namespace langchian::model {
    Ollama::Ollama() {
        Ollama(OLLAMA_ENDPOINT);

    }

    Ollama::Ollama(const langchain::core::Endpoint& endpoint): endpoint(endpoint) {
    }

    langchain::core::LLMResultPtr Ollama::Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) {
    }
} // model
// langchian