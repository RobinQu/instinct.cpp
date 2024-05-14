//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
#define LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerThoughtOutputParser final: public BaseOutputParser<AgentThought> {
    public:
        explicit LLMCompilerThoughtOutputParser(const OutputParserOptions &options)
            : BaseOutputParser<AgentThought>(options) {
        }

        AgentThought ParseResult(const Generation &context) override {
            const auto content = MessageUtils::StringifyGeneration(context);
            static std::regex ACTION_PATTERN {R"(^(\d+)\.\s*(.+)$)"};
            static std::regex DEP_PATTERN {R"(\${?(\d)}?)"};

            AgentThought thought_message;
            auto* tool_call_requests = thought_message.mutable_continuation()->mutable_tool_call_message();

            LLMCompilerTaskGraph graph;
            for(const auto& line: StringUtils::ReSplit(content, std::regex { "\n"})) {
                if (StringUtils::IsBlankString(line)) {
                    // skip blank line
                    continue;
                }
                if (std::smatch idx_match; std::regex_match(line, idx_match, ACTION_PATTERN)) { // action line
                    if (idx_match.length() == 3) {
                        // first item is whole match, second item is matched group and third item is action JSON
                        const auto idx_string = idx_match[1].str();
                        auto idx = std::stol(idx_string);
                        auto* task = graph.mutable_tasks()->Add();
                        task->set_index(idx);
                        const auto action_json_string = idx_match[2].str();
                        auto* tool_call_object = task->mutable_tool_call();
                        ProtobufUtils::Deserialize(action_json_string, *tool_call_object->mutable_function());
                        tool_call_object->set_type(ToolCallObjectType::function);
                        // find deps by parsing arguments string
                        const auto dep_matches = StringUtils::MatchPattern(tool_call_object->mutable_function()->arguments(), DEP_PATTERN);
                        for (const auto& dep_match:dep_matches) {
                            if (dep_match.length() == 2) {
                                auto dep_idx = std::stol(dep_match[1].str());
                                assert_true(dep_idx < idx, "Resolved invalid dependeant task index.");
                                task->add_dependencies(dep_idx);
                            }
                        }

                        if (dep_matches.empty()) { // for the first batch after plan or re-plan, tasks that has no depdencies are selected
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

    static OutputParserPtr<AgentThought> CreateLLMCompilerOutputParser(const OutputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerThoughtOutputParser>(options);
    }
}

#endif //LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
