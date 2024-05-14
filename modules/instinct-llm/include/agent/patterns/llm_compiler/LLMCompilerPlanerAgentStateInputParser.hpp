//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERAGNETSTATEINPUTPARSER_HPP
#define LLMCOMPILERAGNETSTATEINPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "TaskGraphUtils.hpp"
#include "functional/JSONContextPolicy.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "prompt/MessageUtils.hpp"


namespace INSTINCT_LLM_NS {

    class LLMCompilerPlanerAgentStateInputParser final: public BaseInputParser<AgentState> {
    public:
        explicit LLMCompilerPlanerAgentStateInputParser(InputParserOptions options)
            : BaseInputParser<AgentState>(std::move(options)) {
        }

        JSONContextPtr ParseInput(const AgentState &agent_state) override {
            const int n = agent_state.previous_steps_size();
            std::string prompt_input = MessageUtils::ExtractLatestPromptString(agent_state.input());

            // generate tool descriptions
            std::string tool_descriptions;
            for (int i=0; i<agent_state.function_tools_size(); ++i) {
                const auto& function_tool = agent_state.function_tools(i);
                tool_descriptions += fmt::format("{}. {}: {}, arguments JSON schema: {}",  function_tool.name(), function_tool.description(), ProtobufUtils::Serialize(function_tool.parameters()));
            }

            bool replan = agent_state.previous_steps(n-1).thought().has_finish();
            std::string context_string;
            if (replan) {
                for (const auto& step: agent_state.previous_steps()) {
                    if (step.has_thought() && step.thought().has_finish()) {
                        context_string += "Previous Plan: \n\n";
                        const auto previous_joiner_thought = step.thought().finish().response();
                        assert_true(step.thought().finish().details().Is<LLMCompilerTaskGraph>(), "should have LLMCompilerTaskGraph in custom data");
                        LLMCompilerTaskGraph graph;
                        step.thought().finish().details().UnpackTo(&graph);
                        // add previous plan details
                        TaskGraphUtils::BuildAgentScrachPad(graph, context_string);
                        // add joiner thought for previous plan
                        context_string += fmt::format("Thought: {}\n", previous_joiner_thought);
                    }
                }
            }
            context_string += "Current Plan:\n\n";

            // return context
            return CreateJSONContext({
                {"question", prompt_input},
                // plus for for extra `join` function
                {"num_tools", agent_state.function_tools_size() + 1 },
                {"tool_descriptions", tool_descriptions},
                {"repaln", replan},
                {"context", context_string}
            });
        }
    };

    static InputParserPtr<AgentState> CreateLLMCompilerPlanerAgentStateInputParser(const InputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerPlanerAgentStateInputParser>(options);
    }

}


#endif //LLMCOMPILERAGNETSTATEINPUTPARSER_HPP
