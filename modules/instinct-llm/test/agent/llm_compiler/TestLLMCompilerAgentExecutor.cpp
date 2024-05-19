//
// Created by RobinQu on 2024/5/14.
//
#include <gtest/gtest.h>

#include "LLMTestGlobals.hpp"
#include "agent/patterns/llm_compiler/LLMCompilerAgentExecutor.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerAgentExecutorTest: public BaseAgentTest {
    protected:
        void DoSetUp() override {

        }
    };

    TEST_F(LLMCompilerAgentExecutorTest, GenerationTest) {
        const auto executor = CreateLLMCompilerAgentExecutor(chat_model_, {toolkit_});

        AgentState state;
        for(const auto& tool: toolkit_->GetAllFunctionToolSchema()) {
            state.add_function_tools()->CopyFrom(tool);
        }

        auto* msg=  state.mutable_input()->mutable_chat()->add_messages();
        msg->set_content("What's the oldest parrot alive, and how much longer is that than the average?");
        msg->set_role("user");
        const auto step1 = executor->ResolveNextStep(state);
        LOG_INFO("step1={}", step1.ShortDebugString());
        // expect state with thought message
        ASSERT_TRUE(step1.has_thought());
        ASSERT_TRUE(step1.thought().has_continuation());
        ASSERT_TRUE(step1.thought().continuation().tool_call_message().tool_calls_size()>0);
        ASSERT_TRUE(step1.thought().continuation().custom().Is<LLMCompilerTaskGraph>());

        const auto step2 = executor->ResolveNextStep(state);
        LOG_INFO("step2={}", step2.ShortDebugString());
        // expect last step is observation
        ASSERT_TRUE(step2.has_observation());
        ASSERT_TRUE(step2.observation().custom().Is<LLMCompilerTaskGraph>());
        ASSERT_EQ(step2.observation().tool_messages_size(), step1.thought().continuation().tool_call_message().tool_calls_size());
        for(const auto& tool_message: step2.observation().tool_messages()) {
            ASSERT_TRUE(StringUtils::IsNotBlankString(tool_message.content()));
        }

        const auto step3 = executor->ResolveNextStep(state);
        LOG_INFO("step3={}", step3.ShortDebugString());
        // expect last step is finish. answer may not be right though.
        ASSERT_TRUE(step3.has_thought());
        ASSERT_TRUE(step3.thought().has_finish());
        ASSERT_TRUE(step3.thought().finish().custom().Is<LLMCompilerTaskGraph>());
        LLMCompilerTaskGraph graph1;
        step3.thought().finish().custom().UnpackTo(&graph1);
        ASSERT_TRUE(graph1.has_joiner_result());
        ASSERT_FALSE(graph1.joiner_result().is_replan());
        ASSERT_TRUE(StringUtils::IsNotBlankString(step3.thought().finish().response()));
        ASSERT_EQ(graph1.joiner_result().answer(), step3.thought().finish().response());
    }


}
