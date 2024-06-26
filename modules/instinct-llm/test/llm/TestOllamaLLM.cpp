//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>


#include <instinct/llm/OllamaLLM.hpp>
#include <instinct/LLMGlobals.hpp>
#include <instinct/prompt/PlainPromptTemplate.hpp>
#include <instinct/prompt/StringPromptTemplate.hpp>


namespace INSTINCT_LLM_NS {

    class TestOllamaLLM: public testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    static const std::vector<PromptValueVariant> prompts = {
        "Why is sky blue?",
        "What's the biggest country in the world?",
        "What's the population in China as in 2019?"
    };


    TEST_F(TestOllamaLLM, Invoke) {
        const auto ollama_llm = CreateOllamaLLM();
        auto output = ollama_llm->Invoke(prompts[0]);
        std::cout << output << std::endl;

        auto output2 = ollama_llm->Invoke(prompts[0]);
        std::cout << output2 << std::endl;
        ASSERT_TRUE(!output.empty());
    }

    TEST_F(TestOllamaLLM, Batch) {
        const auto ollama_llm = CreateOllamaLLM();
        ollama_llm->Batch(prompts)
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](const auto& t) { LOG_INFO("answer: {}", t); });
    }

    TEST_F(TestOllamaLLM, Stream) {
        const auto ollama_llm = CreateOllamaLLM();
        ollama_llm->Stream(prompts[0])
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](const auto& t) { LOG_INFO("chunk: {}", t); });
    }
}

