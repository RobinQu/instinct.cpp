//
// Created by RobinQu on 2024/1/15.
//

#ifndef OLLAMA_H
#define OLLAMA_H

#include "tools/HttpRestClient.h"
#include <nlohmann/json.hpp>

#include "CoreTypes.h"
#include "model/BaseLLM.h"
#include "model/LLMResult.h"
#include "ModelGlobals.h"
#include "commons/OllamaCommons.h"

namespace LC_MODEL_NS {


class OllamaLLM final: public langchain::core::BaseLLM {
    langchain::core::HttpRestClient http_client_;
public:
    OllamaLLM();
    explicit OllamaLLM(langchain::core::Endpoint endpoint);

protected:
    langchain::core::LLMResultPtr Generate(const std::vector<std::string>& prompts,
        const std::vector<std::string>& stop_words, const langchain::core::OptionDict& options) override;
};

} // model
// langchian

#endif //OLLAMA_H
