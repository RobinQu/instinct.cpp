//
// Created by RobinQu on 2024/4/8.
//

#ifndef REACTINPUTPARSER_HPP
#define REACTINPUTPARSER_HPP
#include "input_parser/BaseInputParser.hpp"
#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;
    class ReACTAgentStateInputParser final: public BaseInputParser<AgentState> {
    public:
        JSONContextPtr ParseInput(const AgentState &agent_state) override {
            std::string scratch_pad;
            for(const auto& step: agent_state.previous_steps()) {
                if (step.has_thought()) {
                    const auto& react_thought = step.thought().react();
                    scratch_pad += "\nThought: " + react_thought.thought();
                    scratch_pad += "\nAction: " + react_thought.invocation().name();
                    scratch_pad += "\nAction Input: " + react_thought.invocation().input();
                }

                if (step.has_observation()) {
                    const auto& react_observation = step.observation().react();
                    // failed invocation should be skipped first
                    assert_true(!react_observation.result().has_error(), "should provide succssful observation");
                    scratch_pad += "\nObservation: " + react_observation.result().return_value();
                }
            }
            scratch_pad += "\nThought: ";

            std::string prompt_input;
            if (agent_state.input().has_chat()) {
                // ignore history messages
                if (const auto msg_count = agent_state.input().chat().messages_size(); msg_count > 0) {
                    prompt_input = agent_state.input().chat().messages(msg_count-1).content();
                }
            }
            if (agent_state.input().has_string()) {
                prompt_input = agent_state.input().string().text();
            }
            assert_true(StringUtils::IsNotBlankString(prompt_input), "should provide valid prompt input");

            auto fn_names = StringUtils::JoinWith(agent_state.function_tools() | std::views::transform([](const FunctionToolSchema& fn_schema) {
                return fn_schema.name();
            }), ",");
            auto tool_desc = RenderFunctionTools(agent_state.function_tools());

            return CreateJSONContext({
                {"agent_scratchpad", scratch_pad},
                {"input", prompt_input},
                {"tool_names", fn_names},
                {"tools", tool_desc}
            });
        }
    };


    static InputParserPtr<AgentState> CreateReACTAgentInputParser() {
        return std::make_shared<ReACTAgentStateInputParser>();
    }
}

#endif //REACTINPUTPARSER_HPP
