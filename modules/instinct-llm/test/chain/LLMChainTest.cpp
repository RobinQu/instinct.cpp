//
// Created by RobinQu on 2024/3/10.
//

#include <gtest/gtest.h>

#include "LLMGlobals.hpp"
#include "chain/LLMChain.hpp"
#include "chat_model/OllamaChat.hpp"
#include "llm/OllamaLLM.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "LLMTestGlobals.hpp"


namespace INSTINCT_LLM_NS {

    class LLMChainTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            chat_model_ = test::create_pesudo_chat_model();
            llm_ = test::create_pesudo_llm();
            chat_prompt_template_ = CreatePlainChatPromptTemplate({
                {kSystem, "You are a help assistant who will try your best to answer user's question."},
                {kHuman, "why is the sky blue?"},
                {kAsisstant, "due to rayleigh scattering."},
                {kHuman, "{question}"}
            });
            string_prompt_template_ = CreatePlainPromptTemplate("{question}");
        }
        ChatModelPtr chat_model_;
        LLMPtr llm_;
        PromptTemplatePtr chat_prompt_template_;
        PromptTemplatePtr string_prompt_template_;
    };


    TEST_F(LLMChainTest, GenerateWithLLM) {
        auto chain = CreateTextChain(
                llm_,
                string_prompt_template_
        );
        auto result = chain->Invoke("Why sky is blue?");
        std::cout << result << std::endl;

        chain->Batch({"Why sky is blue?"})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg); });

        auto chunk_itr = chain->Stream("Why sky is blue?");
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result;
            std::cout << buf << std::endl;;
        }
    }

    TEST_F(LLMChainTest, GenerateWithChatModel) {
        auto pv = "how is that different than mie scattering?";
        auto chain = CreateTextChain(chat_model_, chat_prompt_template_);
        auto result = chain->Invoke(pv);
        std::cout << result << std::endl;

        chain->Batch({pv})
            | rpp::operators::subscribe([](const auto& msg) { LOG_INFO("msg={}", msg); });

        auto chunk_itr = chain->Stream(pv);
        std::string buf;
        for (const auto& chunk_result: CollectVector(chunk_itr)) {
            buf += chunk_result;
            std::cout << buf << std::endl;;
        }
    }

}
