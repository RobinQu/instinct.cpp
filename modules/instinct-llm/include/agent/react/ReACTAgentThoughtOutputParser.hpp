//
// Created by RobinQu on 2024/4/8.
//

#ifndef REACTOUTPUTPARSER_HPP
#define REACTOUTPUTPARSER_HPP


#include <agent.pb.h>

#include "output_parser/BaseOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_LLM_NS;

    struct ReACTAgentStepOutputParserOptions {
        OutputParserOptions base_options = {};
        std::string final_answer_tokens = "Final Answer: ";
        std::string action_name_token = "Action: ";
        std::string action_input_token = "Action Input: ";
        std::string thought_token = "Thought: ";
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

            // assume content before "Action: " is thoguht line.
            if (const auto idx=  content.find(options_.action_name_token); idx != std::string::npos) {
                thought = StringUtils::Trim(content.substr(0, idx));
            }

            // action name and action input should be strictly written in one line or the parser will fail
            for(const auto& line: StringUtils::ReSplit(content, std::regex("\n+"))) {
                if(const auto idx = line.find(options_.action_name_token); idx != std::string::npos) {
                    action_name = line.substr(idx + options_.action_name_token.size());
                }
                if (const auto idx = line.find(options_.action_input_token); idx != std::string::npos) {
                    action_input = line.substr(idx + options_.action_input_token.size());
                }
            }

            // final answer chould have multiple lines
            if (const auto idx = content.find(options_.final_answer_tokens); idx != std::string::npos) {
                final_answer = content.substr(idx + options_.final_answer_tokens.size());
            }

            if (StringUtils::IsNotBlankString(action_name) && StringUtils::IsNotBlankString(thought)) {
                auto* invocation = thought_message.mutable_react()->mutable_invocation();
                invocation->set_name(action_name);
                invocation->set_input(action_input);
                invocation->set_id(StringUtils::GenerateUUIDString());
                thought_message.mutable_react()->set_thought(thought);
                return thought_message;
            }
            if (StringUtils::IsNotBlankString(final_answer)) {
                thought_message.mutable_react()->set_final_answer(final_answer);
                return thought_message;
            }

            // has no thought, no action_name and no final_answer
            LOG_DEBUG("Illegal response for a ReACT agent: {}", content);
            throw InstinctException("Illegal response for a ReACT agent.");
        }

    };

    static OutputParserPtr<AgentThoughtMessage> CreateReACTAgentThoughtOutputParser(const ReACTAgentStepOutputParserOptions& options = {}) {
        return std::make_shared<ReACTAgentThoughtOutputParser>(options);
    }
}

#endif //REACTOUTPUTPARSER_HPP
