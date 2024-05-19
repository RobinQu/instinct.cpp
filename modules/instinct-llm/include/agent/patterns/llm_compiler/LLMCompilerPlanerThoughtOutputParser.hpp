//
// Created by RobinQu on 2024/5/13.
//

#ifndef LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
#define LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP

#include "LLMGlobals.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {

    struct LLMCompilerPlanerThoughtOutputParserOptions {
        OutputParserOptions base_options = {};
    };


    class LLMCompilerPlanerThoughtOutputParser final: public BaseOutputParser<AgentThought> {
    public:
        explicit LLMCompilerPlanerThoughtOutputParser(const LLMCompilerPlanerThoughtOutputParserOptions &options)
            : BaseOutputParser<AgentThought>(options.base_options) {
        }

        AgentThought ParseResult(const Generation &context) override {
            auto content = MessageUtils::StringifyGeneration(context);
            LOG_DEBUG("Planner raw output:\n{}", content);
            static std::regex DEP_PATTERN {R"(\$\{?(\d)\}?)"};
            static std::regex NEW_LINE_SEP { "\n"};
            static std::regex THOUGHT_REGEX {R"(Thought:\s*(.+))"};
            static std::regex ACTION_REGEX {R"((\d+)\.\s*(.+)\(([^\)]*)\))"};

            AgentThought thought_message;
            LLMCompilerTaskGraph graph;

            // clip text if terminal word is found
            if (const auto idx = content.find("<END_OF_PLAN>"); idx != std::string::npos) {
                content = content.substr(0, idx);
            }

            std::string thought_line;
            if (std::smatch thoguht_match; std::regex_search(content, thoguht_match, THOUGHT_REGEX)) {
                if (thoguht_match.size()>=2) {
                    thought_line = thoguht_match[1].str();
                }
            }

            for(const auto& action_match: StringUtils::MatchPattern(content, ACTION_REGEX)) {
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
                }
            }

            if (graph.tasks_size() == 0) { // we turn it into finish step if no tasks are parsed
                LOG_WARN("No tasks found in model output. Return finish step instread");
                thought_message.mutable_finish()->set_response(content);
            } else {
                LOG_DEBUG("Task graph built:\n{}", StringUtils::JoinWith(graph.tasks() | std::views::transform([](const LLMCompilerTaskGraph::LLMCompilerTask& task) {
                    return fmt::format("[index={},name={},deps={},args={}]",
                        task.index(),
                        task.tool_call().function().name(),
                        StringUtils::JoinWith(task.dependencies(), "\n"),
                        task.tool_call().function().arguments()
                    );
                }), ","));
                auto* tool_call_requests = thought_message.mutable_continuation()->mutable_tool_call_message();
                bool found_join = false;
                for (const auto& task: graph.tasks()) {
                    if (task.tool_call().function().name() == "join") {
                        found_join = true;
                    } else {
                        // for the first batch after plan or re-plan, tasks that has no depdencies are selected
                        if (task.dependencies_size() == 0) {
                            tool_call_requests->add_tool_calls()->CopyFrom(task.tool_call());
                        }
                    }
                }
                if (!found_join) {
                    // fill in join if LLM forgets
                    auto* task = graph.add_tasks();
                    task->mutable_tool_call()->set_id(details::generate_next_object_id("call"));
                    task->mutable_tool_call()->set_type(function);
                    task->mutable_tool_call()->mutable_function()->set_name("join");
                    task->set_index(graph.tasks_size());
                    for(int i=1;i<graph.tasks_size();++i) {
                        task->add_dependencies(i);
                    }
                }
                // fill thought line in content
                tool_call_requests->set_content(thought_line);
                // set graph data as custom data on thought
                thought_message.mutable_continuation()->mutable_custom()->PackFrom(graph);
            }
            // return
            return thought_message;
        }
    };

    static OutputParserPtr<AgentThought> CreateLLMCompilerPlanerThoughtOutputParser(const LLMCompilerPlanerThoughtOutputParserOptions& options = {}) {
        return std::make_shared<LLMCompilerPlanerThoughtOutputParser>(options);
    }
}

#endif //LLMCOMPILERTHOUGHTOUTPUTPARSER_HPP
