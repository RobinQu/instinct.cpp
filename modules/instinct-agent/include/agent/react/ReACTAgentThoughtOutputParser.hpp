//
// Created by RobinQu on 2024/4/8.
//

#ifndef REACTOUTPUTPARSER_HPP
#define REACTOUTPUTPARSER_HPP


#include <agent.pb.h>

#include "output_parser/BaseOutputParser.hpp"
#include "AgentGlobals.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;

    struct ReACTAgentStepOutputParserOptions {
        OutputParserOptions base_options = {};
        std::string final_answer_tokens = "Final Answer:";
        std::string action_name_token = "Action:";
        std::string action_input_token = "Action Input:";
        std::string thought_token = "Thought:";
    };

    class ReACTAgentThoughtOutputParser final: public BaseOutputParser<AgentThoughtMessage> {
        ReACTAgentStepOutputParserOptions options_;
    public:
        explicit ReACTAgentThoughtOutputParser(const ReACTAgentStepOutputParserOptions &options)
            : BaseOutputParser(options.base_options), options_(options) {
        }

        AgentThoughtMessage ParseResult(const Generation &context) override {
            AgentThoughtMessage thought_message;
            const auto content = MessageUtils::StringifyGeneration(context);
            std::string action_name, action_input, thought, final_answer;
            for(const auto& line: StringUtils::ReSplit(content, std::regex("\n"))) {
                if (const auto idx = content.find(options_.final_answer_tokens); idx != std::string::npos) {
                    final_answer = line.substr(idx);

                }
                if(const auto idx = content.find(options_.action_name_token) != std::string::npos) {
                    action_name = line.substr(idx);
                }
                if (const auto idx = content.find(options_.action_input_token) != std::string::npos) {
                    action_input = line.substr(idx);
                }

                if (const auto idx = content.find(options_.thought_token) != std::string::npos) {
                    thought = line.substr(idx);
                }
            }
            if (StringUtils::IsBlankString(action_name) || StringUtils::IsBlankString(thought)) {
                throw InstinctException("Illegal response for a ReACT agent.");
            }
            thought_message.mutable_react()->set_final_answer(final_answer);
            auto* invocation = thought_message.mutable_react()->mutable_invocation();
            invocation->set_name(action_name);
            invocation->set_input(action_input);
            invocation->set_id(StringUtils::GenerateUUIDString());
            thought_message.mutable_react()->set_thought(thought);
            return thought_message;
        }

    };

    static OutputParserPtr<AgentThoughtMessage> CreateReACTAgentThoughtOutputParser(const ReACTAgentStepOutputParserOptions& options = {}) {
        return std::make_shared<ReACTAgentThoughtOutputParser>(options);
    }
}

#endif //REACTOUTPUTPARSER_HPP
