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

    struct LLMCompilerPlanerAgentStateInputParserOptions {
        std::string replan_prompt = R"(- You are given "Previous Plan" which is the plan that the previous agent created along with the execution results (given as Observation) of each plan and a general thought (given as Thought) about the executed results. You MUST use these information to create the next plan under "Current Plan".
- When starting the Current Plan, you should start with "Thought" that outlines the strategy for the next plan.
- In the Current Plan, you should NEVER repeat the actions that are already executed in the Previous Plan.
- You must continue the task index from the end of the previous one. Do not repeat task indices.)";
        InputParserOptions base_options;

    };

    class LLMCompilerPlanerAgentStateInputParser final: public BaseInputParser<AgentState> {
        LLMCompilerPlanerAgentStateInputParserOptions options_;
    public:
        explicit LLMCompilerPlanerAgentStateInputParser(LLMCompilerPlanerAgentStateInputParserOptions options)
            : BaseInputParser<AgentState>(std::move(options.base_options)), options_(options) {
        }

        JSONContextPtr ParseInput(const AgentState &agent_state) override {
            const int n = agent_state.previous_steps_size();
            std::string prompt_input = MessageUtils::ExtractLatestPromptString(agent_state.input());

            // generate tool descriptions
            std::string tool_descriptions;
            for (int i=1; i<=agent_state.function_tools_size(); ++i) {
                const auto& function_tool = agent_state.function_tools(i-1);
                tool_descriptions += fmt::format("{}. {}: {}, arguments JSON schema: {}", i,  function_tool.name(), function_tool.description(), ProtobufUtils::Serialize(function_tool.parameters()));
            }

            // to check is_replan preciesely
            bool replan = n>0 && agent_state.previous_steps(n-1).has_observation();
            std::string context_string;
            if (replan) {
                for (const auto& step: agent_state.previous_steps()) {
                    if (step.has_observation()) {
                        context_string += "Previous Plan: \n\n";
                        assert_true(step.observation().custom().Is<LLMCompilerTaskGraph>(), "should have LLMCompilerTaskGraph in custom data");
                        LLMCompilerTaskGraph graph;
                        step.observation().custom().UnpackTo(&graph);
                        // only accept observation with replan flag, as other observation are not complete for whole task graph
                        if (graph.joiner_result().is_replan()) {
                            auto& previous_joiner_thought = graph.joiner_result().response();
                            // add previous plan details
                            TaskGraphUtils::BuildAgentScrachPad(graph, context_string, {.include_action_id = true});
                            // add joiner thought for previous plan
                            context_string += fmt::format("\n\nThought: {}\n\n", previous_joiner_thought);
                        }
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
                {"replan", replan ? options_.replan_prompt: ""},
                {"context", context_string},
                {"exmaples", ""}
            });
        }
    };

    static InputParserPtr<AgentState> CreateLLMCompilerPlanerAgentStateInputParser(const LLMCompilerPlanerAgentStateInputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerPlanerAgentStateInputParser>(options);
    }

}


#endif //LLMCOMPILERAGNETSTATEINPUTPARSER_HPP
