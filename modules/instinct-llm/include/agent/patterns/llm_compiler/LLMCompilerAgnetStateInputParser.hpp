//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERAGNETSTATEINPUTPARSER_HPP
#define LLMCOMPILERAGNETSTATEINPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "functional/JSONContextPolicy.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "prompt/MessageUtils.hpp"


namespace INSTINCT_LLM_NS {

    class LLMCompilerAgnetStateInputParser final: public BaseInputParser<AgentState> {
    public:
        explicit LLMCompilerAgnetStateInputParser(InputParserOptions options)
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

            bool replan = false;
            std::string context_string;
            // if state has last step and it's finish step, then we have to replan
            if (n>0 && agent_state.previous_steps(n-1).thought().has_finish()) {
                replan = true;
                std::unordered_map<std::string, ToolCallObject> tool_calls;
                for (const auto& step: agent_state.previous_steps()) {
                    if (step.has_thought() && step.thought().has_continuation()) {
                        for (const auto& call_object: step.thought().continuation().tool_call_message().tool_calls()) {
                            tool_calls[call_object.id()] = call_object;
                        }
                    }
                    if (step.has_observation()) {
                        context_string += "Previsous Plan: \n\n";
                        for(int i=0;i<step.observation().tool_messages_size();++i) {
                            const auto& tool_result = step.observation().tool_messages(i);
                            if (tool_calls.contains(tool_result.tool_call_id())) {
                                const auto& tool_call = tool_calls.at(tool_result.tool_call_id());
                                context_string += fmt::format("{}. {}", i, ProtobufUtils::Serialize(tool_call.function()));
                                context_string += fmt::format("Observation: {}\n", tool_result.content());
                            } else {
                                LOG_WARN("Unmatched observation for tool call id: {}", tool_result.tool_call_id());
                            }
                        }
                    }
                    context_string += "\n\n";
                }
            }

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

    static InputParserPtr<AgentState> CreateLLMCompilerInputParser(const InputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerAgnetStateInputParser>(options);
    }

}


#endif //LLMCOMPILERAGNETSTATEINPUTPARSER_HPP
