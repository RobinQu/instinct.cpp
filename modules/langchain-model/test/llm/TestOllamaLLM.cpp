//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>

#include "llm/OllamaLLM.h"


TEST(OllamaLLM, TestDerivedClasss) {
    LC_MODEL_NS::OllamaGenerateRequest request;
    nlohmann::json req = request;
    std::cout << req << std::endl;

    LC_MODEL_NS::OllamaGenerateResponse response;
    nlohmann::json res = response;
    std::cout << res << std::endl;
    response = res.get<LC_MODEL_NS::OllamaGenerateResponse>();

}

TEST(OllamaLLM, TestSimpleGenerate) {
    LC_MODEL_NS::OllamaLLM ollama_llm;
    auto output = ollama_llm.Predict("Why sky is blue?", {}, {});
    std::cout << output << std::endl;
}

TEST(OllamaLM, TestStream) {

}