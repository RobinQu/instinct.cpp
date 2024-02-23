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

    TEST(OllamaLM, TestStream) {

    }
}

