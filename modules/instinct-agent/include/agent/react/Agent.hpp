//
// Created by RobinQu on 2024/4/8.
//

#ifndef AGENT_HPP
#define AGENT_HPP


#include <agent.pb.h>

#include "AgentGlobals.hpp"
#include "LLMGlobals.hpp"
#include "chain/LLMChain.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "functional/StepFunctions.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"
#include "functional/Xn.hpp"
#include "agent/react/ReACTInputParser.hpp"
#include "agent/react/ReACTOutputParser.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;
    /**
     * Wondering if Agent class should exist?
     * planner: a Chain that takes in a prompt and genreate action steps
     * toolkits: a sequence of toolkits to solve specific tasks
     *
     * Input: AgentSatet
     * Output: AgentStepList
     * OutputChunk: AgentStep
     */
    class IAgent {

    };

    /**
    * Simple Agent that uses ReACT strategy to solve a specific problem
    * Input: AgentState
    * Output: AgentStep
    */
    static MessageChainPtr<AgentState, AgentStep> CreateReACTTextChain(const ChatModelPtr& chat_model) {
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
        chat_model->Configure({.stop_words = {"\nObservation"} });
        const auto input_parser = CreateReACTAgentInputParser();
        const auto output_parser = CreateReACTOutputParser();
        return CreateLLMChain(input_parser, chat_model, output_parser, chat_prompt_tempate);
    }

}


#endif //AGENT_HPP
