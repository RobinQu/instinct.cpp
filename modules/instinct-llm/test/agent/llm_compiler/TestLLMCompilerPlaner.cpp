//
// Created by RobinQu on 2024/5/14.
//
#include <gtest/gtest.h>
#include "LLMTestGlobals.hpp"
#include "agent/patterns/llm_compiler/LLMCompilerPlanerAgentStateInputParser.hpp"
#include "agent/patterns/llm_compiler/LLMCompilerPlanerThoughtOutputParser.hpp"


namespace INSTINCT_LLM_NS {
    class LLMCompilerPlanTest: public BaseAgentTest {
    protected:
        void DoSetUp() override {
            SetupLogging();
        }
    };

    TEST_F(LLMCompilerPlanTest, ParseInputState) {
        auto input_parser = CreateLLMCompilerPlanerAgentStateInputParser();

        // empty state
        AgentState state;
        for(const auto& tool: toolkit_->GetAllFunctionToolSchema()) {
            state.add_function_tools()->CopyFrom(tool);
        }

        auto* msg = state.mutable_input()->mutable_chat()->add_messages();
        msg->set_content("How much more expensive of gold price in Hongkong compared to that in New York?");
        msg->set_role("human");
        const auto ctx1 = input_parser->ParseInput(state);
        ASSERT_EQ(ctx1->RequireMappingData().at("question")->RequirePrimitive<std::string>(), msg->content());
        ASSERT_EQ(ctx1->RequireMappingData().at("num_tools")->RequirePrimitive<int>(), 3);

        // replan with observation
        // 1. mock a graph composed of three tasks
        ToolCallObject tool_call_1;
        tool_call_1.set_id(details::generate_next_object_id("call"));
        tool_call_1.mutable_function()->set_name("search");
        tool_call_1.mutable_function()->set_arguments(R"({"query":"gold price in Hongkong"})");
        ToolCallObject tool_call_2;
        tool_call_2.set_id(details::generate_next_object_id("call"));
        tool_call_2.mutable_function()->set_name("search");
        tool_call_2.mutable_function()->set_arguments(R"({"query":"gold price in New York"})");
        ToolCallObject tool_call_3;
        tool_call_3.set_id(details::generate_next_object_id("call"));
        tool_call_3.mutable_function()->set_name("calculate");
        tool_call_3.mutable_function()->set_arguments(R"({"question":"What's #1 minus #2?"})");
        ToolCallObject tool_call_4;
        tool_call_4.set_id(details::generate_next_object_id("call"));
        tool_call_4.mutable_function()->set_name("join");
        LLMCompilerTaskGraph graph;
        auto* task1 = graph.add_tasks();
        task1->set_index(1);
        task1->mutable_tool_call()->CopyFrom(tool_call_1);
        auto* task2 = graph.add_tasks();
        task2->set_index(2);
        task2->mutable_tool_call()->CopyFrom(tool_call_2);
        auto* task3 = graph.add_tasks();
        task3->set_index(3);
        task3->mutable_tool_call()->CopyFrom(tool_call_3);
        task3->add_dependencies(1);
        task3->add_dependencies(2);
        auto* task4 = graph.add_tasks();
        task4->mutable_tool_call()->CopyFrom(tool_call_4);
        task4->add_dependencies(1);
        task4->add_dependencies(2);
        task4->add_dependencies(3);

        // 2. create thought and obsercation message with two tool calls and
        auto* thought_step = state.add_previous_steps();
        auto* continuation = thought_step->mutable_thought()->mutable_continuation();
        continuation->mutable_tool_call_message()->add_tool_calls()->CopyFrom(tool_call_1);
        continuation->mutable_tool_call_message()->add_tool_calls()->CopyFrom(tool_call_2);
        continuation->mutable_custom()->PackFrom(graph);
        auto* observation_step = state.add_previous_steps();
        auto *tool_result1 = observation_step->mutable_observation()->add_tool_messages();
        tool_result1->set_tool_call_id(tool_call_1.id());
        tool_result1->set_content("11 USD for 1 Gram");
        tool_result1->set_role("tool");
        auto *tool_result2 = observation_step->mutable_observation()->add_tool_messages();
        tool_result2->set_tool_call_id(tool_call_2.id());
        tool_result2->set_content("10 USD for 1 Gram");
        tool_result2->set_role("tool");
        task1->mutable_result()->CopyFrom(*tool_result1);
        task2->mutable_result()->CopyFrom(*tool_result2);
        observation_step->mutable_observation()->mutable_custom()->PackFrom(graph);

        // 3. create thought and observation with third call
        auto* thought_step2 = state.add_previous_steps();
        auto* continuation2 = thought_step2->mutable_thought()->mutable_continuation();
        continuation2->mutable_tool_call_message()->add_tool_calls()->CopyFrom(tool_call_3);
        continuation2->mutable_custom()->PackFrom(graph);
        auto* observation_step2 = state.add_previous_steps();
        task3->mutable_result()->set_content(R"(Failed to calculate: What's '11 USD for 1 Gram' minus '10 USD for 1 Gram'. Please provide float numbers as input.)");
        observation_step2->mutable_observation()->add_tool_messages()->CopyFrom(task3->result());
        graph.mutable_joiner_result()->set_is_replan(true);
        graph.mutable_joiner_result()->set_response("I have to do the final math with numbers");
        observation_step2->mutable_observation()->mutable_custom()->PackFrom(graph);

        // 4. run replan
        const auto ctx2 = input_parser->Invoke(state);
        ASSERT_TRUE(ctx2->RequireMappingData().at("replan")->RequirePrimitive<bool>());
        const auto context1 = ctx2->RequireMappingData().at("context")->RequirePrimitive<std::string>();
        LOG_INFO("context={}", context1);
        ASSERT_TRUE(StringUtils::IsNotBlankString(context1));
        //
        //
        // // 3. mock new task graph
        // LLMCompilerTaskGraph graph2;
        // auto* task5 = graph2.add_tasks();
        // ToolCallObject tool_call_5;
        // tool_call_5.set_id(details::generate_next_object_id("call"));
        // tool_call_5.set_type(function);
        // tool_call_5.mutable_function()->set_name("calculate");
        // tool_call_5.mutable_function()->set_arguments("What's 11 minus 10?");
        // task5->mutable_tool_call()->CopyFrom(tool_call_5);
        // task5->set_index(1);
        // ToolCallObject tool_call_6;
        // tool_call_6.set_id(details::generate_next_object_id("call"));
        // tool_call_6.mutable_function()->set_name("join");
        // auto* task6 = graph2.add_tasks();
        // task6->mutable_tool_call()->CopyFrom(tool_call_6);
        // task6->set_index(2);
        //
        // // 4. create thought and observation after replan
        // auto* thought_step3 = state.add_previous_steps();
        // auto* continuation3 = thought_step3->mutable_thought()->mutable_continuation();
        // continuation3->mutable_custom()->PackFrom(graph2);
        // continuation3->mutable_tool_call_message()->set_content("I have to do the final math with numbers"); // this is joiner's thought
        // auto* observation_step_3 = state.add_previous_steps();
        // task5->mutable_result()->set_content("1");
        // observation_step_3->mutable_observation()->add_tool_messages()->CopyFrom(task5->result());
        // observation_step_3->mutable_observation()->mutable_custom()->PackFrom(graph2);
        //
    }

    TEST_F(LLMCompilerPlanTest, ParseOutputThought) {
        auto output_parser = CreateLLMCompilerPlanerThoughtOutputParser();
        Generation generation;
        generation.set_text(R"(1. search({"query": "gold price in Hongkong"})
2. search({"query": "gold price in New York"})
3. calculate({"question": "What's $1 minus $2?"})
4. join()
)");
        const auto thought = output_parser->ParseResult(generation);
        ASSERT_TRUE(thought.continuation().custom().Is<LLMCompilerTaskGraph>());
        LLMCompilerTaskGraph graph;
        thought.continuation().custom().UnpackTo(&graph);
        ASSERT_EQ(graph.tasks_size(), 4);
        ASSERT_EQ(graph.tasks(0).tool_call().function().name(), "search");
        ASSERT_EQ(graph.tasks(1).tool_call().function().name(), "search");
        ASSERT_EQ(graph.tasks(2).tool_call().function().name(), "calculate");
        ASSERT_EQ(graph.tasks(2).tool_call().function().arguments(), R"({"question": "What's $1 minus $2?"})");
        ASSERT_EQ(to_vector(graph.tasks(2).dependencies()), std::vector<int64_t>({1,2} ));
        ASSERT_EQ(graph.tasks(3).tool_call().function().name(), "join");
        ASSERT_EQ(to_vector(graph.tasks(3).dependencies()), std::vector<int64_t>({1,2,3} ));
    }





}
