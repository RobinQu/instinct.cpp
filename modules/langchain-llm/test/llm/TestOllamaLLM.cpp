//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>


#include "llm/OllamaLLM.hpp"
#include "ModelGlobals.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "prompt/StringPromptTemplate.hpp"


LC_LLM_NS {

    static const std::vector<core::LanguageModelInput> prompts = {
        "Why is sky blue?",
        "What's the biggest country in the world?",
        "What's the population in China as in 2019?"
    };

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
        auto output = ollama_llm(prompts[0]);
        std::cout << output << std::endl;


        auto string_prompt = core::StringPromptValue("Why is sky blue? ");
        auto output2 = ollama_llm.GeneratePrompts({string_prompt}, {});
        ASSERT_EQ(output2.generations.size(), 1);
        ASSERT_EQ(output2.generations[0].size(), 1);
        ASSERT_TRUE(std::holds_alternative<core::Generation>(output2.generations[0][0]));
    }

    TEST(OllamaLM, TestBatch) {

        OllamaLLM ollama_llm;
        auto* result = ollama_llm.Batch(prompts);
        while(result->HasNext()) {
            std::cout << result->Next() << std::endl;
        }
        delete result;
    }

    TEST(OllamaLLM, TestStream) {
        OllamaLLM ollama_llm;
        auto* result = ollama_llm.Stream(prompts[0]);
        while (result->HasNext()) {
            std::cout << result->Next() << std::endl;
        }
        delete result;
    }
}

