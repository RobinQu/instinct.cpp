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
                .model_name = "local-model",
                .endpoint = {.host = "localhost", .port = 3928}
            });
        }

        ChatModelPtr openai_chat;
    };


    TEST_F(OpenAIChatTest, SimpleGenerate) {
        const auto result = openai_chat->Invoke("why sky is blue?");
        std::cout << result << std::endl;
    }
}