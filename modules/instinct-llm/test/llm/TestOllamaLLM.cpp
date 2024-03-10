//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>


#include "llm/OllamaLLM.hpp"
#include "LLMGlobals.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "prompt/StringPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {

    static const std::vector<PromptValueVariant> prompts = {
        "Why is sky blue?",
        "What's the biggest country in the world?",
        "What's the population in China as in 2019?"
    };


    TEST(TestOllamaLLM, SimpleGenerate) {
        OllamaLLM ollama_llm;
        auto output = ollama_llm(prompts[0]);
        std::cout << output << std::endl;

        auto output2 = ollama_llm.Invoke(prompts[0]);
        std::cout << output2 << std::endl;
        ASSERT_TRUE(!output.empty());
    }

    TEST(TestOllamaLLM, Batch) {
        OllamaLLM ollama_llm;
        auto result = ollama_llm.Batch(prompts);
        while(result->HasNext()) {
            std::cout << result->Next() << std::endl;
        }
        delete result;
    }

    TEST(TestOllamaLLM, Stream) {
        OllamaLLM ollama_llm;
        auto result = ollama_llm.Stream(prompts[0]);
        while (result->HasNext()) {
            std::cout << result->Next() << std::endl;
        }
        delete result;
    }
}

