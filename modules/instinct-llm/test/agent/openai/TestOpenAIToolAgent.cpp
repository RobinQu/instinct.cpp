//
// Created by RobinQu on 2024/4/30.
//
#include <gtest/gtest.h>
#include "LLMGlobals.hpp"
#include "../../../include/instinct/LLMTestGlobals.hpp"
#include "agent/patterns/openai_tool/OpenAIToolAgentExecutor.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "toolkit/LocalToolkit.hpp"

namespace INSTINCT_LLM_NS {
    class OpenAIToolAgentTest: public testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
        }
        ChatModelPtr chat_model = CreateOpenAIChatModel({
            .model_name = "gpt-3.5-turbo"
        });
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
