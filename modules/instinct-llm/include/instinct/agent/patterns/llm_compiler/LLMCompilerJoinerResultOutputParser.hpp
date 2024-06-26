//
// Created by RobinQu on 2024/5/14.
//

#ifndef LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
#define LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
#include <instinct/LLMGlobals.hpp>
#include <instinct/output_parser/BaseOutputParser.hpp>
#include <instinct/prompt/MessageUtils.hpp>

namespace INSTINCT_LLM_NS {


    struct LLMCompilerJoinerResultOutputParserOptions {
        std::string action_token = "Action:";
        std::string thought_token = "Thought:";
        std::string replan_action_name = "Replan";
        std::string answer_for_unknown = "Unknown";
        bool replan_enabled = false;
        OutputParserOptions base_options = {};
    };

    class LLMCompilerJoinerResultOutputParser final: public BaseOutputParser<LLMCompilerJoinerResult> {
        LLMCompilerJoinerResultOutputParserOptions options_;
    public:
        explicit LLMCompilerJoinerResultOutputParser(const LLMCompilerJoinerResultOutputParserOptions &options)
            : BaseOutputParser<LLMCompilerJoinerResult>(options.base_options), options_(options) {
        }

        LLMCompilerJoinerResult ParseResult(const Generation &context) override {
            static std::regex THOUGHT_REGEX {R"(Thought:\s*(.+))"};
            static std::regex ACTION_REGEX {R"(Action:\s*(.+)\(([^\)]*)\))"};
            const auto content = MessageUtils::StringifyGeneration(context);
            LLMCompilerJoinerResult joiner_result;

            if (std::smatch thought_match; std::regex_search(content, thought_match, THOUGHT_REGEX)) {
                if (thought_match.size()>=2) {
                    joiner_result.set_thought(thought_match[1]);
                }
            }
            if (std::smatch action_match; std::regex_search(content, action_match, ACTION_REGEX)) {
                if (action_match.size() >= 3) {
                    // item one is matched line
                    // item two is action name
                    const auto action = action_match[1].str();
                    joiner_result.set_is_replan(action.find(options_.replan_action_name) != std::string::npos);
                    // item three is answered
                    joiner_result.set_answer(action_match[2].str());
                }
            }

            // normalize joiner result
            if (!joiner_result.is_replan() && StringUtils::IsBlankString(joiner_result.answer())) {
                joiner_result.set_answer(options_.answer_for_unknown);
            }

            return joiner_result;
        }
    };

    static OutputParserPtr<LLMCompilerJoinerResult> CreateLLMCompilerJoinerResultOutputParser(const LLMCompilerJoinerResultOutputParserOptions &options = {}) {
        return std::make_shared<LLMCompilerJoinerResultOutputParser>(options);
    }


}

#endif //LLMCOMPILERJOINERRESULTOUTPUTPARSER_HPP
