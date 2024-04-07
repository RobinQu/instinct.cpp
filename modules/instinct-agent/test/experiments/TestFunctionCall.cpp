//
// Created by RobinQu on 2024/4/6.
//
#include <gtest/gtest.h>

#include "chat_model/OllamaChat.hpp"
#include "chain/LLMChain.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"


/**
 * This test mick simple agent with function calling
 */
TEST(TestFunctionCall, SimpleCall) {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    const auto chat_prompt_tempate = CreatePlainChatPromptTemplate({
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

    const auto ollama_chat = CreateOllamaChatModel({.model_name = "llama2:latest"});
    const auto chain = chat_prompt_tempate | ollama_chat->AsModelFunction();
    const auto ctx = CreateJSONContext({
        {"input", "目前市场上玫瑰花的平均价格是多少？如果我在此基础上加价15%卖出，应该如何定价？"},
        {"agent_scratchpad", ""},
        {"tools", tool_desc},
        {"tool_names", tool_names}
    });
    const auto result = chain->Invoke(ctx);
    LOG_INFO("result = {}", SanitizeJSONContext(result).dump());
}