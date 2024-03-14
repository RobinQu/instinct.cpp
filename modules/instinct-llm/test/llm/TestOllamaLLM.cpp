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


    TEST(TestOllamaLLM, Invoke) {
        OllamaLLM ollama_llm;
        auto output = ollama_llm(prompts[0]);
        std::cout << output << std::endl;

        auto output2 = ollama_llm.Invoke(prompts[0]);
        std::cout << output2 << std::endl;
        ASSERT_TRUE(!output.empty());
    }

    TEST(TestOllamaLLM, Batch) {
        OllamaLLM ollama_llm;

        ollama_llm.Batch(prompts)
            | rpp::operators::subscribe([](const auto& t) { LOG_INFO("answer: {}", t); });
    }

    TEST(TestOllamaLLM, Stream) {
        OllamaLLM ollama_llm;
        ollama_llm.Stream(prompts[0])
            | rpp::operators::subscribe([](const auto& t) { LOG_INFO("chunk: {}", t); });
    }
}

