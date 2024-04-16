//
// Created by RobinQu on 2024/3/15.
//
#include <gtest/gtest.h>

#include "LLMGlobals.hpp"
#include "chat_model/OpenAIChat.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * this test expects a locally running OpenAI-compatible server.
     */
    class OpenAIChatTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            openai_chat = CreateOpenAIChatModel({
                                                        .endpoint = {.host = "192.168.0.134", .port = 8000},
                                                        .model_name = "TheBloke/Mixtral-8x7B-Instruct-v0.1-GPTQ",
            });
        }

        ChatModelPtr openai_chat;
    };


    TEST_F(OpenAIChatTest, SimpleGenerate) {
        const auto result = openai_chat->Invoke("why sky is blue?");
        std::cout << result << std::endl;

        openai_chat->Batch(std::vector<PromptValueVariant> {
            "why sky is blue?",
            "How many counties are in America?"
        })
        | rpp::operators::subscribe([](const auto& msg) {std::cout << msg << std::endl; });


    }

    TEST_F(OpenAIChatTest, StreamGenerate) {
        openai_chat->Stream("What's capital city of France?")
        // | rpp::operators::as_blocking()
        | rpp::operators::subscribe([](const auto& m) { std::cout << "output message: " <<  m << std::endl; });
    }



}