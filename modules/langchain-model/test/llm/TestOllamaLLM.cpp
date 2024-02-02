//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>

#include "llm/OllamaLLM.h"

TEST(OllamaLLM, TestSimpleGenerate) {
    langchian::model::OllamaLLM ollama_llm;
    auto output = ollama_llm.Predict("Hello", {}, {});
    std::cout << output << std::endl;
}