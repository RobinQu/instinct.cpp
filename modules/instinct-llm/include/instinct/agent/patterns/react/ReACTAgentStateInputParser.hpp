//
// Created by RobinQu on 2024/4/8.
//

#ifndef REACTINPUTPARSER_HPP
#define REACTINPUTPARSER_HPP
#include "input_parser/BaseInputParser.hpp"
#include "LLMGlobals.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_LLM_NS;
    class ReACTAgentStateInputParser final: public BaseInputParser<AgentState> {
    public:
        explicit ReACTAgentStateInputParser(InputParserOptions options)
            : BaseInputParser<AgentState>(std::move(options)) {
        }

        JSONContextPtr ParseInput(const AgentState &agent_state) override {
            std::string scratch_pad;
            const int n = agent_state.previous_steps_size();

            // zero or more thought and observation paris. last step cannot be thought message.
            for(int i=0; i<n; ++i) {
                const auto& step = agent_state.previous_steps(i);
                if (step.has_thought()) {
                    const auto& react_thought = step.thought().continuation();
                    scratch_pad += react_thought.tool_call_message().content();
                    const auto& function_object = react_thought.tool_call_message().tool_calls(0).function();
                    scratch_pad += "\nAction: " + function_object.name();
                    scratch_pad += "\nAction Input: " + function_object.arguments();
                }

                if (step.has_observation()) {
                    const auto& react_observation = step.observation();
                    // failed invocation should be skipped first
                    assert_true(react_observation.tool_messages_size()>0, "should provide succssful observation");
                    scratch_pad += "\nObservation: " + react_observation.tool_messages(0).content();
                    scratch_pad += "\nThought: ";
                }
            }
            // scratch_pad += "\nThought: ";

            std::string prompt_input = MessageUtils::ExtractLatestPromptString(agent_state.input());
            assert_true(StringUtils::IsNotBlankString(prompt_input), "should provide valid prompt input");
            auto fn_names = StringUtils::JoinWith(agent_state.function_tools() | std::views::transform([](const FunctionTool& fn_schema) {
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


    static InputParserPtr<AgentState> CreateReACTAgentInputParser(const InputParserOptions& options = {}) {
        return std::make_shared<ReACTAgentStateInputParser>(options);
    }
}

#endif //REACTINPUTPARSER_HPP
