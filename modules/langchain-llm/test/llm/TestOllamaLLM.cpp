//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>


#include "llm/OllamaLLM.hpp"
#include "ModelGlobals.hpp"


LC_LLM_NS {
    TEST(OllamaLLM, TestDerivedClasss) {
        OllamaGenerateRequest request;
        nlohmann::json req = request;
        std::cout << req << std::endl;

        OllamaGenerateResponse response;
        nlohmann::json res = response;
        std::cout << res << std::endl;
        response = res.get<OllamaGenerateResponse>();

    }

    TEST(OllamaLLM, TestSimpleGenerate) {
        OllamaLLM ollama_llm;
        auto output = ollama_llm("Why sky is blue?");
        std::cout << output << std::endl;
    }

    TEST(OllamaLM, TestBatch) {
        const std::vector<core::LanguageModelInput> prompts = {
            "Why sky is blue?",
            "What's the biggest country in the world?",
            "What's the population in China as in 2019?"
        };
        OllamaLLM ollama_llm;
        for(const auto& result: ollama_llm.Batch(prompts, {})) {
            std::cout << result << std::endl;
        }
    }
}

