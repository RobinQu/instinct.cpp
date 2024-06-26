//
// Created by RobinQu on 2024/5/14.
//

#ifndef LOCALTOOLKITSWORKER_HPP
#define LOCALTOOLKITSWORKER_HPP

#include <instinct/agent/BaseWorker.hpp>
#include <instinct/LLMGlobals.hpp>

namespace INSTINCT_LLM_NS {
    class LocalToolkitsWorker final: public BaseWorker {
        ThreadPool thread_pool_;

    public:
        explicit LocalToolkitsWorker(const std::vector<FunctionToolkitPtr> &toolkits)
            : BaseWorker(toolkits) {
        }

        AgentObservation Invoke(const AgentThought &input) override {
                const auto &tool_request_msg = input.continuation().tool_call_message();

                std::vector<ToolCallObject> filtered_tool_calls;
                for (const auto& call: tool_request_msg.tool_calls()) {
                    for (const auto &tk: GetFunctionToolkits()) {
                        if (tk->LookupFunctionTool({.by_name = call.function().name()})) {
                            filtered_tool_calls.push_back(call);
                        }
                    }
                }

                AgentObservation observation;
                // it's possible we have empty tool calls after fitering
                if (!filtered_tool_calls.empty()) {
                    // only execute tool call that has matching tools in worker
                    for (auto multi_futures = thread_pool_.submit_sequence(0, tool_request_msg.tool_calls_size(), [&](auto i) {
                        const auto &call = filtered_tool_calls.at(i);
                        for (const auto &tk: GetFunctionToolkits()) {
                            if (tk->LookupFunctionTool({.by_name = call.function().name()})) {
                                return tk->Invoke(call);
                            }
                        }
                        // impossible to reach here
                        throw InstinctException(fmt::format("Unresolved invocation: id={}, name={}", call.id(),
                                                            call.function().name()));
                    }); auto &future: multi_futures) {
                        const auto tool_result = future.get();
                        if (tool_result.has_error()) {
                            LOG_ERROR("invocation failed: id={}, exception={}", tool_result.invocation_id(), tool_result.exception());
                            throw InstinctException(tool_result.exception());
                        }
                        Message function_message;
                        function_message.set_role("tool");
                        function_message.set_tool_call_id(tool_result.invocation_id());
                        function_message.set_content(tool_result.return_value());
                        observation.add_tool_messages()->CopyFrom(function_message);
                    }
                }
                return observation;
            }

    };


    static WorkerPtr CreateLocalToolkitsWorker(const std::vector<FunctionToolkitPtr> &toolkits) {
        return std::make_shared<LocalToolkitsWorker>(toolkits);
    }
}


#endif //LOCALTOOLKITSWORKER_HPP
