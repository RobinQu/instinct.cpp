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
     * TODO should seperate unit tests and integration tests. use mock server for unit tests instread.
     */
    class OpenAIChatTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }

        ChatModelPtr CreateOpenAIChat() {
            return CreateOpenAIChatModel();
        }

        ChatModelPtr CreateLLMStudioChat() {
            return CreateOpenAIChatModel({
                                                        .endpoint = {.host = "192.168.0.134", .port = 8000},
                                                        .model_name = "TheBloke/Mixtral-8x7B-Instruct-v0.1-GPTQ",
            });
        }
    };

    //
    // TEST_F(OpenAIChatTest, SimpleGenerate) {
    //     const auto openai_chat = CreateLLMStudioChat();
    //     const auto result = openai_chat->Invoke("why sky is blue?");
    //     std::cout << result << std::endl;
    //
    //     openai_chat->Batch(std::vector<PromptValueVariant> {
    //         "why sky is blue?",
    //         "How many counties are in America?"
    //     })
    //     | rpp::operators::subscribe([](const auto& msg) {std::cout << msg << std::endl; });
    // }
    //
    // TEST_F(OpenAIChatTest, StreamGenerate) {
    //     const auto openai_chat = CreateLLMStudioChat();
    //     openai_chat->Stream("What's capital city of France?")
    //     // | rpp::operators::as_blocking()
    //     | rpp::operators::subscribe([](const auto& m) { std::cout << "output message: " <<  m << std::endl; });
    // }

    TEST_F(OpenAIChatTest, ToolUses) { // test requires OPENAI_APIKEY and SERP_APIKEY envs
        const auto openai_chat = CreateOpenAIChat();
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
            if (msg.tool_calls_size() > 0) {
                // add tool_call message
                messages.add_messages()->CopyFrom(msg);
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
        ASSERT_EQ(message.role(), "assistant");
        ASSERT_FALSE(message.content().empty());
        ASSERT_TRUE(message.content().find("Tesla") != std::string::npos);
    }


}
