//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
#define LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerPlanerThoughtOutputParser final: public BaseOutputParser<AgentThought> {
    public:
        explicit LLMCompilerPlanerThoughtOutputParser(const OutputParserOptions &options)
            : BaseOutputParser<AgentThought>(options) {
        }

        AgentThought ParseResult(const Generation &context) override {
            const auto content = MessageUtils::StringifyGeneration(context);
            static std::regex ACTION_PATTERN {R"(^(\d+)\.\s*(.+)\((.*)\)$)"};
            static std::regex DEP_PATTERN {R"(\$\{?(\d)\}?)"};
            static std::regex NEW_LINE_SEP { "\n"};

            AgentThought thought_message;
            auto* tool_call_requests = thought_message.mutable_continuation()->mutable_tool_call_message();

            LLMCompilerTaskGraph graph;
            for(const auto& line: StringUtils::ReSplit(content, NEW_LINE_SEP)) {
                if (StringUtils::IsBlankString(line)) {
                    // skip blank line
                    continue;
                }
                if (std::smatch action_match; std::regex_match(line, action_match, ACTION_PATTERN)) { // action line
                    if (action_match.size() >= 3) {
                        // first item is whole match, second item is matched group and third item is action JSON
                        const auto idx_string = action_match[1].str();
                        auto idx = std::stol(idx_string);
                        auto* task = graph.mutable_tasks()->Add();
                        task->set_index(idx);
                        const auto action_name_string = action_match[2].str();

                        auto* tool_call_object = task->mutable_tool_call();
                        tool_call_object->mutable_function()->set_name(action_name_string);
                        if (action_match.size() >= 4) {
                            tool_call_object->mutable_function()->set_arguments(action_match[3].str());
                        }
                        tool_call_object->set_type(function);
                        tool_call_object->set_id(details::generate_next_object_id("call"));
                        // find deps by parsing arguments string
                        const auto dep_matches = StringUtils::MatchPattern(tool_call_object->mutable_function()->arguments(), DEP_PATTERN);
                        for (const auto& dep_match: dep_matches) {
                            if (dep_match.size() == 2) {
                                auto dep_idx = std::stol(dep_match[1].str());
                                assert_true(dep_idx < idx, "Resolved invalid dependeant task index.");
                                task->add_dependencies(dep_idx);
                            }
                        }

                        if (action_name_string == "join") {
                            for(int i=1;i<idx;++i) {
                                task->add_dependencies(i);
                            }
                            break;
                        }

                        // for the first batch after plan or re-plan, tasks that has no depdencies are selected
                        if (dep_matches.empty()) {
                            tool_call_requests->add_tool_calls()->CopyFrom(*tool_call_object);
                        }
                    }
                }
            }

            // set graph data as custom data on thought
            thought_message.mutable_continuation()->mutable_custom()->PackFrom(graph);

            // return
            return thought_message;
        }
    };

    static OutputParserPtr<AgentThought> CreateLLMCompilerPlanerThoughtOutputParser(const OutputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerPlanerThoughtOutputParser>(options);
    }
}

#endif //LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
