//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H
#include <LangchainCore.h>

namespace langchian {
namespace model {

class OllamaLLM final: langchain::core::BaseLLM {
    langchain::core::Endpoint endpoint;
public:
    OllamaLLM();
    explicit OllamaLLM(const langchain::core::Endpoint& endpoint);

protected:
    langchain::core::LLMResultPtr Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) override;
};

} // model
} // langchian

#endif //OLLAMA_H
