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

    struct ReACTOutputParserOptions {
        OutputParserOptions base_options = {};
        std::string final_answer_tokens = "Final Answer:";
        std::string action_name_token = "Action:";
        std::string action_input_token = "Action Input:";
        std::string thought_token = "Thought:";
    };

    class ReACTOutputParser final: public BaseOutputParser<AgentStep> {
        ReACTOutputParserOptions options_;
    public:
        explicit ReACTOutputParser(const ReACTOutputParserOptions &options)
            : BaseOutputParser(options.base_options), options_(options) {
        }

        AgentStep ParseResult(const Generation &context) override {
            AgentStep step;
            const auto content = MessageUtils::StringifyGeneration(context);
            std::string action_name, action_input, thought;
            for(const auto& line: StringUtils::ReSplit(content, std::regex("\n"))) {
                if (const auto idx = content.find(options_.final_answer_tokens); idx != std::string::npos) {
                    step.mutable_finish()->set_response(line.substr(idx));
                    return step;
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
            auto* thought_message = step.mutable_react_thought();
            thought_message->set_thought(thought);
            auto* invocation = thought_message->mutable_invocation();
            invocation->set_name(action_name);
            invocation->set_input(action_input);
            invocation->set_id(StringUtils::GenerateUUIDString());
            return step;
        }

    };

    static OutputParserPtr<AgentStep> CreateReACTOutputParser(const ReACTOutputParserOptions& options = {}) {
        return std::make_shared<ReACTOutputParser>(options);
    }
}

#endif //REACTOUTPUTPARSER_HPP
