//
// Created by RobinQu on 2024/3/15.
//
#include <gtest/gtest.h>

#include "LLMGlobals.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "toolkit/LocalToolkit.hpp"
#include "toolkit/builtin/LLMMath.hpp"
#include "toolkit/builtin/SerpAPI.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * this test expects a locally running OpenAI-compatible server.
     * TODO should separate unit tests and integration tests. use mock server for unit tests instread.
     */
    class OpenAIChatTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

    };


     TEST_F(OpenAIChatTest, SimpleGenerate) {
         const auto openai_chat = CreateOpenAIChatModel();
         const auto result = openai_chat->Invoke("why sky is blue?");
         std::cout << result << std::endl;

         openai_chat->Batch(std::vector<PromptValueVariant> {
             "why sky is blue?",
             "How many counties are in America?"
         })
         | rpp::operators::subscribe([](const auto& msg) {std::cout << msg << std::endl; });
     }

     TEST_F(OpenAIChatTest, StreamGenerate) {
         const auto openai_chat = CreateOpenAIChatModel();
         openai_chat->Stream("What's capital city of France?")
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](const auto& m) { LOG_INFO("output={}", m.ShortDebugString()); });
     }

    TEST_F(OpenAIChatTest, ToolUses) { // test requires OPENAI_APIKEY and SERP_APIKEY envs
        const auto openai_chat = CreateOpenAIChatModel();
        // const auto calculator = CreateLLMMath(openai_chat);
        const auto serp_api = CreateSerpAPI();
        const auto toolkit = CreateLocalToolkit({ serp_api});
        openai_chat->BindTools(toolkit);

        MessageList messages;
        llm::Message message;
        message.set_content("Why Elon Musk have an urgent visit to China in April. 2024?");
        message.set_role("user");
        messages.add_messages()->CopyFrom(message);
        while (true) {
            const auto msg = openai_chat->Invoke(messages);
            LOG_INFO("msg=[{}]", msg.ShortDebugString());
            messages.add_messages()->CopyFrom(msg);
            if (msg.tool_calls_size() > 0) {
                // add tool_call message
                for(const auto& call: msg.tool_calls()) {
                    ToolCallObject invocation;
                    invocation.set_id(call.id());
                    invocation.mutable_function()->set_name(call.function().name());
                    invocation.mutable_function()->set_arguments(call.function().arguments());
                    const auto result = toolkit->Invoke(invocation);
                    llm::Message function_message;
                    if (result.has_error()) {
                        throw InstinctException(fmt::format("function call failed: {}", result.exception()));
                    }
                    // add tool result message
                    function_message.set_role("tool");
                    function_message.set_tool_call_id(call.id());
                    function_message.set_content(result.return_value());
                    messages.add_messages()->CopyFrom(function_message);
                }
            } else {
                break;
            }
        }
        auto last_msg = *messages.messages().rbegin();
        ASSERT_EQ(last_msg.role(), "assistant");
        ASSERT_FALSE(last_msg.content().empty());
        ASSERT_TRUE(last_msg.content().find("Elon") != std::string::npos);
    }


}
