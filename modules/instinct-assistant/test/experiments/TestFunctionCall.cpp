//
// Created by RobinQu on 2024/4/6.
//
#include <gtest/gtest.h>

#include "chat_model/OpenAIChat.hpp"
#include "chain/LLMChain.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"


/**
 * function to mimic a search engine
 * @param query
 * @return
 */
static std::string search_web(const std::string& query) {
    return "The average price for a dozen roses in the U.S. is $80.16. The state where a dozen roses cost the most is Hawaii at $108.33. That\'s 35% more expensive than the national average. A dozen roses are most affordable in Pennsylvania, costing $66.15 on average.";
}

/**
 * a function to mimick a calculatorr
 * @param expression
 * @return
 */
static std::string calculate(const std::string& expression) {
    return std::to_string(rand() * 10);
}


/**
 * This test mick simple agent with function calling
 */
TEST(TestFunctionCall, SimpleCall) {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    SetupLogging();
    const auto chat_prompt_template = CreatePlainChatPromptTemplate({
        {kSystem,  R"(Answer the following questions as best you can. You have access to the following tools:

{tools}

Use the following format:

Question: the input question you must answer
Thought: you should always think about what to do
Action: the action to take, should be one of [{tool_names}]
Action Input: the input to the action, which is formatted as JSON blob with 'name' and 'arguments' keys.
Observation: the result of the action
... (this Thought/Action/Action Input/Observation can repeat N times)
Thought: I now know the final answer
Final Answer: the final answer to the original input question

Begin!

Question: {input}
Thought:{agent_scratchpad})"}
    });

    std::string tool_desc = R"(
        search_web: A search engine. Useful for when you need to answer questions about current events. args: {"query": { "type": "string"}}
        calculator: A tool that answer question about math calculation. {"math_expression": {"type": "string"}}
    )";

    std::string tool_names = "search_web, calculator";

    std::string action_input_literal = "Action Input:";

    const auto chat_model = CreateOpenAIChatModel({.stop_words = {"\nObservation"}});

    const auto chain = chat_prompt_template | chat_model->AsModelFunction() | xn::steps::stringify_generation();

    std::string scratch_pad;
    while (true) {
        auto ctx = CreateJSONContext({
            {"input", "目前市场上玫瑰花的平均价格是多少？如果我在此基础上加价15%卖出，应该如何定价？"},
            {"agent_scratchpad", scratch_pad},
            {"tools", tool_desc},
            {"tool_names", tool_names}
        });
        const auto result = chain->Invoke(ctx);
        const auto content = result->RequirePrimitive<std::string>();
        LOG_INFO("iteration result: {}", content);
        if (content.find("Final Answer") != std::string::npos) {
            break;
        }
        scratch_pad += content;

        const auto args_string = content.substr(content.find(action_input_literal) + action_input_literal.size());
        LOG_INFO("args string: {}", args_string);
        const auto args = nlohmann::json::parse(args_string);
        if (content.find("search_web") != std::string::npos) {
            scratch_pad += fmt::format("\nObservation: {tool_result}\nThought:",
                fmt::arg (
                    "tool_result",
                    search_web(args["query"].get<std::string>()).c_str()
                )
            );
        }
        if (content.find("calculator") != std::string::npos) {
            scratch_pad += fmt::format("\nObservation: {tool_result}\nThought:",
                fmt::arg(
                    "tool_result",
                    calculate(args["math_expression"].get<std::string>()).c_str()
                )
            );
        }
    }


}