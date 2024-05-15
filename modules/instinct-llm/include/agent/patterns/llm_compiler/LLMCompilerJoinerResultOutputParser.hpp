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
            static std::regex NEWLINE_SEP {"\n"};
            static std::regex ACTION_REGEX {R"(Action:\s*(.+)\((.*)\))"};
            const auto content = MessageUtils::StringifyGeneration(context);
            LLMCompilerJoinerResult joiner_result;
            for (const auto& line: StringUtils::ReSplit(content, NEWLINE_SEP)) {
                auto trimmed_line = StringUtils::Trim(line);
                if (StringUtils::IsBlankString(trimmed_line)) {
                    continue;
                }
                if (trimmed_line.starts_with(options_.thought_token)) {
                    const auto thought = StringUtils::Trim(trimmed_line.substr(options_.thought_token.size()));
                    joiner_result.set_thought(thought);
                }
                if (std::smatch action_match; std::regex_match(trimmed_line, action_match, ACTION_REGEX)) {
                    if (action_match.size() >= 3) {
                        // item one is matched line
                        // item two is action name
                        const auto action = action_match[1].str();
                        joiner_result.set_is_replan(action.find(options_.replan_action_name) != std::string::npos);
                        // item three is answer
                        joiner_result.set_answer(action_match[2].str());
                    }
                }
            }
            return joiner_result;
        }
    };

    static OutputParserPtr<LLMCompilerJoinerResult> CreateLLMCompilerJoinerResultOutputParser(const LLMCompilerJoinerResultOutputParserOptions &options = {}) {
        return std::make_shared<LLMCompilerJoinerResultOutputParser>(options);
    }


}

#endif //LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
