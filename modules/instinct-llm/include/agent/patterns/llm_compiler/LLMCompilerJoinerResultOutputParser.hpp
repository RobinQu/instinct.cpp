//
// Created by RobinQu on 2024/5/14.
//

#ifndef LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
#define LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
#include "LLMGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {


    struct LLMCompilerJoinerResultOutputParserOptions {
        std::string action_token = "Action:";
        std::string thought_token = "Thought:";
        std::string replan_action_name = "Replan";
        OutputParserOptions base_options = {};
    };

    class LLMCompilerJoinerResultOutputParser final: public BaseOutputParser<LLMCompilerJoinerResult> {
        LLMCompilerJoinerResultOutputParserOptions options_;
    public:
        explicit LLMCompilerJoinerResultOutputParser(const LLMCompilerJoinerResultOutputParserOptions &options)
            : BaseOutputParser<LLMCompilerJoinerResult>(options.base_options), options_(options) {
        }

        LLMCompilerJoinerResult ParseResult(const Generation &context) override {
            static std::regex NEWLINE_SEP({"\n"});
            const auto content = MessageUtils::StringifyGeneration(context);
            std::string thought;
            bool is_replan = false;
            for (const auto& line: StringUtils::ReSplit(content, NEWLINE_SEP)) {
                if (line.starts_with(options_.action_token)) {
                    const auto begin_idx = line.find_first_of("(");
                    const auto end_idx = line.find_last_of(")");
                    if (begin_idx != std::string::npos && end_idx != std::string::npos) {
                        const auto action = line.substr(begin_idx+1, end_idx);
                        is_replan = action.find(options_.replan_action_name) != std::string::npos;
                    }
                }
                if (line.starts_with(options_.thought_token)) {
                    thought = line.substr(options_.thought_token.size());
                }
            }

            LLMCompilerJoinerResult joiner_result;
            joiner_result.set_response(thought);
            joiner_result.set_is_replan(is_replan);
            return joiner_result;
        }
    };


}

#endif //LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
