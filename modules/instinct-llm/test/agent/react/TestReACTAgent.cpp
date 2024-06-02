//
// Created by RobinQu on 2024/4/9.
//
#include <gtest/gtest.h>

#include "LLMTestGlobals.hpp"
#include "agent/patterns/react/Agent.hpp"
#include "chat_model/OllamaChat.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "toolkit/LocalToolkit.hpp"
#include "toolkit/builtin/LLMMath.hpp"
#include "toolkit/builtin/SerpAPI.hpp"
#include "tools/SystemUtils.hpp"

namespace INSTINCT_LLM_NS {
    class ReACTAgentTest: public BaseAgentTest {
    protected:
        void SetUp() override {
            SetupLogging();
            // relies on OpenAI or other auto-configured service
            chat_model_ = CreateOpenAIChatModel({
                .temperature = 0,
                .stop_words = { {"Observation:" }}
            });
        }

        ChatModelPtr chat_model_;
    };

    TEST_F(ReACTAgentTest, ParseAgentStateInput) {
        const auto agent_executor = CreateReACTAgentExecutor(
            chat_model_,
            {toolkit_}
        );

        const std::string prompt = "How to make a paper plane?";
        auto state = agent_executor->InitializeState(prompt);
        const auto input_parse = CreateReACTAgentInputParser();

        // empty state
        const auto ctx1 = input_parse->ParseInput(state);
        ASSERT_EQ(ctx1->RequireMappingData().at("input")->RequirePrimitive<std::string>(), prompt);
        ASSERT_EQ(ctx1->RequireMappingData().at("agent_scratchpad")->RequirePrimitive<std::string>(), "");
        ASSERT_EQ(ctx1->RequireMappingData().at("tools")->RequirePrimitive<std::string>(), "Search: A search engine. Useful for when you need to answer questions about current events. Input should be a search query. arg={\"type\":\"object\",\"properties\":{\"query\":{\"type\":\"string\"}}}\nCalculator: Useful for when you need to answer questions about math. arg={\"type\":\"object\",\"properties\":{\"math_question\":{\"type\":\"string\"}}}");
        ASSERT_EQ(ctx1->RequireMappingData().at("tool_names")->RequirePrimitive<std::string>(), "Search,Calculator");


        // with thought and observation
        auto* step = state.add_previous_steps();
        auto* react_thought = step->mutable_thought()->mutable_continuation();
        react_thought->mutable_tool_call_message()->set_content(R"(The user is asking for instructions on how to make a paper airplane. This is not a factual query, so I cannot provide a direct answer from my knowledge base.
However, I can look up a reliable source for such instructions. I will use the "search" tool to find a trusted guide on making a basic paper airplane.)");
        // react_thought->mutable_tool_call_message()->mutable_custom()->PackFrom(custom_data);
        auto* tool_call = react_thought->mutable_tool_call_message()->add_tool_calls();
        tool_call->mutable_function()->set_name("Search");
        tool_call->mutable_function()->set_arguments(R"({"query": "how to make a simple paper airplane"}{start_of_response})");
        step = state.add_previous_steps();
        auto* react_observation = step->mutable_observation()->mutable_tool_messages()->Add();
        react_observation->set_content(R"(The top result is a Wikipedia How-To guide on origami, which includes steps to make a simple paper airplane.

1. Start with a rectangular piece of paper, such as an A4 or letter size sheet. If it's not already, fold it in half lengthwise and then unfold it.
2. Fold the top two corners down so they meet the center crease. This will make the top point of the plane.
3. Fold the new top point down to where the paper is double-folded (from step 2). You should now have a folded piece that looks like an envelope.
4. Fold the top two corners in to the center again, so they meet along the middle crease.
5. Fold the plane in half toward you along the center crease, keeping the folds you just made aligned.
6. Make sure the edges are even and folded neatly, then fold the wings down so their top edges line up with the bottom of the plane's body.
7. Adjust the wings to make sure they are symmetrical, and make any necessary adjustments to ensure the plane flies straight.
8. Hold the airplane under its wings and throw it gently but firmly straight ahead.
)");

        const auto ctx2 = input_parse->ParseInput(state);
        LOG_INFO("scratch_pad: {}", ctx2->RequireMappingData().at("agent_scratchpad")->RequirePrimitive<std::string>());
        ASSERT_EQ(ctx2->RequireMappingData().at("agent_scratchpad")->RequirePrimitive<std::string>(), R"(The user is asking for instructions on how to make a paper airplane. This is not a factual query, so I cannot provide a direct answer from my knowledge base.
However, I can look up a reliable source for such instructions. I will use the "search" tool to find a trusted guide on making a basic paper airplane.
Action: Search
Action Input: {"query": "how to make a simple paper airplane"}{start_of_response}
Observation: The top result is a Wikipedia How-To guide on origami, which includes steps to make a simple paper airplane.

1. Start with a rectangular piece of paper, such as an A4 or letter size sheet. If it's not already, fold it in half lengthwise and then unfold it.
2. Fold the top two corners down so they meet the center crease. This will make the top point of the plane.
3. Fold the new top point down to where the paper is double-folded (from step 2). You should now have a folded piece that looks like an envelope.
4. Fold the top two corners in to the center again, so they meet along the middle crease.
5. Fold the plane in half toward you along the center crease, keeping the folds you just made aligned.
6. Make sure the edges are even and folded neatly, then fold the wings down so their top edges line up with the bottom of the plane's body.
7. Adjust the wings to make sure they are symmetrical, and make any necessary adjustments to ensure the plane flies straight.
8. Hold the airplane under its wings and throw it gently but firmly straight ahead.

Thought: )");
    }

    TEST_F(ReACTAgentTest, ParseAgentThought) {
        const auto thought_parser = CreateReACTAgentThoughtOutputParser();
        std::string output = "This question appears to be asking for the current average price of roses in the market, and then what the price should be if one were to increase it by 15%. I will assume that the question is being asked in Chinese, with \"目前市场上\" translating to \"current market\", \"玫瑰花\" translating to \"roses\", and \"平均价格\" translating to \"average price\".\n\nAction: Search\nAction Input: {\"query\":\"current average price of roses in the market\", \"result_limit\":1, \"result_offset\":0}\nObservation:";

        Generation generation;
        generation.set_text(output);
        const auto thought1 = thought_parser->ParseResult(generation);
        ASSERT_TRUE(thought1.has_continuation() && thought1.continuation().has_tool_call_message());
        ASSERT_TRUE(thought1.continuation().tool_call_message().tool_calls_size()>0);
        ASSERT_EQ(thought1.continuation().tool_call_message().content(), "This question appears to be asking for the current average price of roses in the market, and then what the price should be if one were to increase it by 15%. I will assume that the question is being asked in Chinese, with \"目前市场上\" translating to \"current market\", \"玫瑰花\" translating to \"roses\", and \"平均价格\" translating to \"average price\".");
        ASSERT_EQ(thought1.continuation().tool_call_message().tool_calls(0).function().name(), "Search");
        ASSERT_EQ(thought1.continuation().tool_call_message().tool_calls(0).function().arguments(), "{\"query\":\"current average price of roses in the market\", \"result_limit\":1, \"result_offset\":0}");
    }

    TEST_F(ReACTAgentTest, StreamSteps) {
        const auto agent_executor = CreateReACTAgentExecutor(
            chat_model_,
            {toolkit_}
        );
        const auto state = agent_executor->InitializeState("目前市场上玫瑰花的平均价格是多少？如果我在此基础上加价15%卖出，应该如何定价？");
        agent_executor->Stream(state)
            | rpp::operators::subscribe([](const AgentState& new_state) {
                auto& latest_step = *new_state.previous_steps().rbegin();
                LOG_INFO("Progressed: {}", latest_step.ShortDebugString());
            });
    }

    TEST_F(ReACTAgentTest, GetFinalStep) {
        const auto agent_executor = CreateReACTAgentExecutor(
            chat_model_,
            {toolkit_}
        );
        // this question won't trigger tool use
        const auto state = agent_executor->InitializeState("How to make a paper plane?");
        const auto final_state = agent_executor->Invoke(state);
        LOG_INFO("final state: {}", final_state.ShortDebugString());
        ASSERT_TRUE(final_state.previous_steps().rbegin()->has_thought() && final_state.previous_steps().rbegin()->thought().has_finish());
        ASSERT_FALSE(final_state.previous_steps().rbegin()->thought().finish().is_failed());
    }


}
