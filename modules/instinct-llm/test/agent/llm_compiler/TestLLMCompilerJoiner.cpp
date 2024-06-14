//
// Created by RobinQu on 2024/5/14.
//
#include <gtest/gtest.h>
#include "LLMTestGlobals.hpp"
#include "agent/patterns/llm_compiler/LLMCompilerJoiner.hpp"


namespace INSTINCT_LLM_NS {
    class LLMCompilerJoinerTest: public BaseAgentTest {
    protected:
        void DoSetUp() override {

        }
    };

    TEST_F(LLMCompilerJoinerTest, TestInputParser) {
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
        tool_call_3.mutable_function()->set_name("join");

        LLMCompilerTaskGraph graph;
        graph.set_question("How much more expensive of gold price in Hongkong compared to that in New York?");
        auto* task1 = graph.add_tasks();
        task1->set_index(1);
        task1->mutable_tool_call()->CopyFrom(tool_call_1);
        auto* task2 = graph.add_tasks();
        task2->set_index(2);
        task2->mutable_tool_call()->CopyFrom(tool_call_2);
        auto* task3 = graph.add_tasks();
        task3->set_index(3);
        task3->mutable_tool_call()->CopyFrom(tool_call_3);
        auto *tool_result1 = task1->mutable_result();
        tool_result1->set_tool_call_id(tool_call_1.id());
        tool_result1->set_content("11 USD for 1 Gram");
        tool_result1->set_role("tool");
        auto *tool_result2 = task2->mutable_result();
        tool_result2->set_tool_call_id(tool_call_2.id());
        tool_result2->set_content("10 USD for 1 Gram");
        tool_result2->set_role("tool");
        task1->mutable_result()->CopyFrom(*tool_result1);
        task2->mutable_result()->CopyFrom(*tool_result2);
        const auto input_parser = CreateLLMCompilerJoinerTaskGraphInputParser();
        const auto ctx1 = input_parser->Invoke(graph);
        ASSERT_EQ(ctx1->RequireMappingData().at("question")->RequirePrimitive<std::string>(), graph.question());
        const auto scratchpad = ctx1->RequireMappingData().at("agent_scratchpad")->RequirePrimitive<std::string>();
        LOG_INFO("scratchpad: {}", scratchpad);
        ASSERT_TRUE(StringUtils::IsNotBlankString(scratchpad));
    }

    TEST_F(LLMCompilerJoinerTest, TestParseFinishAction) {
        Generation generation;
        generation.set_text(R"(Thought: I know the final answer.
        Action: Finish(It's one dollar more expensive in Hongkong.)
)");
        const auto output_parser = CreateLLMCompilerJoinerResultOutputParser();
        const LLMCompilerJoinerResult result = output_parser->ParseResult(generation);
        ASSERT_FALSE(result.is_replan());
        ASSERT_EQ(result.thought(), "I know the final answer.");
        ASSERT_EQ(result.answer(), "It's one dollar more expensive in Hongkong.");
    }

    TEST_F(LLMCompilerJoinerTest, TestParseReplanAction) {
        Generation generation;
        generation.set_text(R"(Thought: something went wrong.
        Action: Replan()
)");
        const auto output_parser = CreateLLMCompilerJoinerResultOutputParser();
        const LLMCompilerJoinerResult result = output_parser->ParseResult(generation);
        ASSERT_TRUE(result.is_replan());
        ASSERT_EQ(result.thought(), "something went wrong.");
        ASSERT_TRUE(StringUtils::IsBlankString(result.answer()));
    }

    TEST_F(LLMCompilerJoinerTest, TestJoin) {
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
        tool_call_3.mutable_function()->set_name("join");

        LLMCompilerTaskGraph graph;
        graph.set_question("How much more expensive of gold price in Hongkong compared to that in New York?");
        auto* task1 = graph.add_tasks();
        task1->set_index(1);
        task1->mutable_tool_call()->CopyFrom(tool_call_1);
        auto* task2 = graph.add_tasks();
        task2->set_index(2);
        task2->mutable_tool_call()->CopyFrom(tool_call_2);
        auto* task3 = graph.add_tasks();
        task3->set_index(3);
        task3->mutable_tool_call()->CopyFrom(tool_call_3);
        auto *tool_result1 = task1->mutable_result();
        tool_result1->set_tool_call_id(tool_call_1.id());
        tool_result1->set_content("11 USD for 1 Gram");
        tool_result1->set_role("tool");
        auto *tool_result2 = task2->mutable_result();
        tool_result2->set_tool_call_id(tool_call_2.id());
        tool_result2->set_content("10 USD for 1 Gram");
        tool_result2->set_role("tool");
        task1->mutable_result()->CopyFrom(*tool_result1);
        task2->mutable_result()->CopyFrom(*tool_result2);

        const auto joiner = CreateLLMCompilerJoiner(chat_model_);
        const auto result = joiner->Invoke(graph);
        LOG_INFO("joiner_result={}", result.ShortDebugString());
        ASSERT_FALSE(result.is_replan());
    }

}
