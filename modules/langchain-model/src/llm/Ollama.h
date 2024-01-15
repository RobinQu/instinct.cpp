//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H
#include <LangchainCore.h>

namespace langchian {
namespace model {

class Ollama: langchain::core::BaseLanguageModel {
    langchain::core::Endpoint endpoint;
public:
    Ollama() = default;
    explicit Ollama(langchain::core::Endpoint& endpoint);


};

} // model
} // langchian

#endif //OLLAMA_H
