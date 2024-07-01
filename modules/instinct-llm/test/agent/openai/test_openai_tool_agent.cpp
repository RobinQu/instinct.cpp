//
// Created by RobinQu on 2024/4/30.
//
#include <gtest/gtest.h>
#include <instinct/llm_global.hpp>
#include <instinct/llm_test_global.hpp>
#include <instinct/agent/patterns/openai_tool/openai_tool_agent_executor.hpp>
#include <instinct/agent/executor/agent_executor.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/toolkit/local_toolkit.hpp>

namespace INSTINCT_LLM_NS {
    class OpenAIToolAgentTest: public testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
        }
        ChatModelPtr chat_model = CreateOpenAIChatModel();
        FunctionToolkitPtr tool_kit = CreateLocalToolkit(
            std::make_shared<GetFlightPriceTool>(),
            std::make_shared<GetNightlyHotelPrice>()
        );
    };


    /**
     * A use case extracted from: https://gist.github.com/gaborcselle/5d9e45232319b543ca5336f8ab23e8d2
     **/
    TEST_F(OpenAIToolAgentTest, NormalExecution) {
        const auto agent_executor = CreateOpenAIToolAgentExecutor(chat_model, {tool_kit});
        const auto initial_state = agent_executor->InitializeState("How much would a 3 day trip to New York, Paris, and Tokyo cost?");
        agent_executor->Stream(initial_state)
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([](const AgentState& state) {
                auto& latest_step = *state.previous_steps().rbegin();
                if (latest_step.thought().has_finish()) {
                    LOG_INFO("Final answer: {}", latest_step.thought().finish().response());
                }
            })
        ;
    }
}
