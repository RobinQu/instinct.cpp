//
// Created by RobinQu on 2024/5/13.
//

#ifndef TASKGRAPHUTILS_HPP
#define TASKGRAPHUTILS_HPP

#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * a utility class that handles task fetching
     */
    class TaskGraphUtils final {
    public:

        static void FindNextTasks(const LLMCompilerTaskGraph& graph, std::vector<int64_t>& next_task_ids) {
            std::unordered_set<int64_t> finished;
            for (const auto& task: graph.tasks()) {
                if (task.has_result() && StringUtils::IsNotBlankString(task.result().content())) {
                    finished.insert(task.index());
                }
            }
            for (const auto& task: graph.tasks()) {
                if (!finished.contains(task.index())) {
                    const bool match = std::ranges::all_of(task.dependencies(), [&](const auto& idx) {
                       return finished.contains(idx);
                    });
                    if (match || task.dependencies_size() == 0) { // has no deps or all deps are fulfilled
                        next_task_ids.push_back(task.index());
                    }
                }
            }
        }


        static void BuildToolCallRequest(const LLMCompilerTaskGraph& graph, const std::vector<int64_t>& next_task_ids, Message* tool_call_request) {
            std::unordered_map<int64, int> id_index;
            for(int i=0;i<graph.tasks_size();++i) {
                const auto& task = graph.tasks(i);
                id_index[task.index()] = i;
            }
            for(const auto& id: next_task_ids) {
                assert_true(id_index.contains(id), fmt::format("Assigned task id should exist in graph. id={}", id));
                auto &task = graph.tasks(id_index[id]);
                tool_call_request->add_tool_calls()->CopyFrom(task.tool_call());
                tool_call_request->set_role("assistant");
            }
        }

        struct ScrtchPadFormatOptions {
            bool inlcude_aciton = true;
            bool include_thgouth = true;
            bool include_action_id = false;
        };

        static void BuildAgentScrachPad(const LLMCompilerTaskGraph& graph, std::string& output, const ScrtchPadFormatOptions& options = {}) {
            for (const auto& task: graph.tasks()) {
                if (options.include_thgouth && StringUtils::IsNotBlankString(task.thought())) {
                    output += fmt::format("Thought: {}\n", task.thought());
                }
                const auto idx = options.include_action_id ? fmt::format("{}.", task.index()) : "";
                if (options.inlcude_aciton) {
                    output += fmt::format("{}{} {}\n", idx, task.tool_call().function().name(),  task.tool_call().function().arguments());
                }
                if (StringUtils::IsNotBlankString(task.result().content())) {
                    output += fmt::format("Observation: {}\n", task.result().content());
                }
            }
        }
    };
}


#endif //TASKGRAPHUTILS_HPP
