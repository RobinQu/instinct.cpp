//
// Created by RobinQu on 2024/5/8.
//

#ifndef LLMCOMPILERAGENTEXECUTOR_HPP
#define LLMCOMPILERAGENTEXECUTOR_HPP

#include <utility>

#include "LLMCompilerJoiner.hpp"
#include "LLMCompilerPlaner.hpp"
#include "LLMGlobals.hpp"
#include "TaskGraphUtils.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "agent/patterns/openai_tool/OpenAIToolAgentExecutor.hpp"

namespace INSTINCT_LLM_NS {

    struct LLMCompilerOptions {
        int max_replan = 6; // max count for running replaner
        LLMCompilerPlanerAgentStateInputParserOptions planer_input_parser = {};
        LLMCompilerPlanerThoughtOutputParserOptions planer_output_parser = {};
        LLMCompilerJoinerResultOutputParserOptions joiner_output_parser = {};
        LLMCompilerJoinerTaskGraphInputParserOptions joiner_input_parser = {};
    };

    class LLMCompilerAgentExecutor final: public BaseAgentExecutor {
        StopPredicate should_early_stop_;
        PlannerPtr planner_;
        WorkerPtr worker_;
        JoinerPtr joiner_;
        LLMCompilerOptions options_;

    public:
        LLMCompilerAgentExecutor(StopPredicate should_early_stop, PlannerPtr planner, WorkerPtr worker, JoinerPtr joiner, LLMCompilerOptions  options)
            : should_early_stop_(std::move(should_early_stop)),
              planner_(std::move(planner)),
              worker_(std::move(worker)),
              joiner_(std::move(joiner)),
              options_(std::move(options)) {
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            AgentStep agent_step;
            // check if early stop is required
            if (should_early_stop_(state, agent_step)) {
                return agent_step;
            }

            const auto n = state.previous_steps_size();
            if (n == 0) {
                // do initial planing
                const AgentThought thought_step = planner_->Invoke(state);
                agent_step.mutable_thought()->CopyFrom(thought_step);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }
            const auto& last_step = state.previous_steps(n - 1);

            // here's the tricky part to make task graph and joiner work
            if(last_step.has_observation()) {
                // recover task graph
                LLMCompilerTaskGraph graph;
                assert_true(last_step.observation().custom().Is<LLMCompilerTaskGraph>(), "should have LLMCompilerTaskGraph as custom data in the observation step");
                last_step.observation().custom().UnpackTo(&graph);

                std::vector<int64_t> next_ids;
                TaskGraphUtils::FindNextTasks(graph, next_ids);
                if (!next_ids.empty()) { // current function graph is not finished, we have to generate another thought to continue
                    LOG_INFO("Found executable tasks in graph. ids={}, graph={}",
                        StringUtils::JoinWith(next_ids, ","),
                        graph.ShortDebugString()
                    );
                    auto* tool_call_requests = agent_step.mutable_thought()->mutable_continuation()->mutable_tool_call_message();
                    TaskGraphUtils::BuildToolCallRequest(graph, next_ids, tool_call_requests);
                    // save task graph data in the new thought step
                    agent_step.mutable_thought()->mutable_continuation()->mutable_custom()->PackFrom(graph);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }

                // the function graph is exhausted, we can run joiner
                // hack1: set question for joiner manually
                graph.set_question(MessageUtils::ExtractLatestPromptString(state.input()));
                const auto joiner_result = joiner_->Invoke(graph);
                // hack2: save joiner thought in graph
                graph.mutable_joiner_result()->CopyFrom(joiner_result);
                state.mutable_previous_steps()->rbegin()->mutable_observation()->mutable_custom()->PackFrom(graph);
                if (joiner_result.is_replan()) {
                    // if we have to replan, we should plan again and return thought message for continuation
                    AgentThought thought_step = planner_->Invoke(state);
                    assert_true(thought_step.continuation().custom().Is<LLMCompilerTaskGraph>(), "should have LLMCompilerTaskGraph as custom data in the thought message");
                    agent_step.mutable_thought()->CopyFrom(thought_step);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }

                // agent has final answer, so we could directly return
                auto* finish_step = agent_step.mutable_thought()->mutable_finish();
                finish_step->set_response(joiner_result.answer());
                // copy graph data in finish step
                finish_step->mutable_custom()->PackFrom(graph);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            if (last_step.has_thought()
                && last_step.thought().has_pause()
                && last_step.thought().pause().has_tool_call_message()
                    ) {
                // for pause step, user should submit rest of tool results through IRunService::SubmitToolOutputs.
                // so we just check if all tools are done here
                const auto& pause = last_step.thought().pause();
                assert_true(pause.custom().Is<LLMCompilerTaskGraph>(), "should contain LLMCompilerTaskGraph in custom data in the pause step");

                if (pause.tool_call_message().tool_calls_size() == pause.completed_size()) {
                    // update user submitted tool call result into graph
                    LLMCompilerTaskGraph graph;
                    pause.custom().UnpackTo(&graph);
                    for (const auto& tool_message: pause.completed()) {
                        for (auto& task: *graph.mutable_tasks()) {
                            if (task.tool_call().id() == tool_message.tool_call_id()) {
                                task.mutable_result()->CopyFrom(tool_message);
                            }
                        }
                    }
                    // lift to observation
                    agent_step.mutable_observation()->mutable_tool_messages()->CopyFrom(pause.completed());
                    // copy task graph data
                    agent_step.mutable_observation()->mutable_custom()->PackFrom(graph);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }
            }

            // if last step is paused or finished, it cannot be executed again
            if (last_step.has_thought()
                    && last_step.thought().has_continuation()
                    && last_step.thought().continuation().has_tool_call_message()
                    && last_step.thought().continuation().tool_call_message().tool_calls_size() > 0
                    ) {
                const auto& tool_call_message = last_step.thought().continuation().tool_call_message();
                const auto& tool_call_objects = tool_call_message.tool_calls();
                const auto& thought_step = last_step.thought();
                assert_true(thought_step.continuation().custom().Is<LLMCompilerTaskGraph>(), "should contain LLMCompilerTaskGraph in custom data in the thought step");
                LLMCompilerTaskGraph graph;
                thought_step.continuation().custom().UnpackTo(&graph);
                assert_gt(graph.tasks_size(), 1, "There should be more than one task in LLMCompilerTaskGraph");

                // worker will take care of execution of built-in tools
                const auto observation_message = worker_->Invoke(thought_step);
                int completed = 0;
                for(const auto& tool_call: tool_call_objects) {
                    for(const auto& tool_message: observation_message.tool_messages()) {
                        if (tool_message.tool_call_id() == tool_call.id()) {
                            completed++;
                        }
                    }
                }

                // update result in task graph
                for(auto& task: *graph.mutable_tasks()) {
                    for(const auto& tool_message: observation_message.tool_messages()) {
                        if (tool_message.tool_call_id() == task.tool_call().id()) {
                            task.mutable_result()->CopyFrom(tool_message);
                        }
                    }
                }

                // obviously some of tools are handled to users and we return a pause step to wait for tool results
                if (completed != tool_call_objects.size()) {
                    auto* pause = agent_step.mutable_thought()->mutable_pause();
                    pause->mutable_tool_call_message()->CopyFrom(tool_call_message);
                    pause->mutable_completed()->CopyFrom(observation_message.tool_messages());
                    // save task graph to custom
                    pause->mutable_custom()->PackFrom(graph);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }

                agent_step.mutable_observation()->CopyFrom(observation_message);
                agent_step.mutable_observation()->mutable_custom()->PackFrom(graph);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            LOG_DEBUG("illegal state: {}", state.ShortDebugString());
            throw InstinctException("IllegalState for LLMCompilerAgentExectuor");
        }
    };

    static AgentExecutorPtr CreateLLMCompilerAgentExecutor(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits,
        const StopPredicate& stop_predicate = NoStopPredicate,
        const LLMCompilerOptions& options = {}
        ) {
        auto planer = CreateLLMCompilerPlaner(chat_model, options.planer_input_parser, options.planer_output_parser);
        auto worker = CreateLocalToolkitsWorker(toolkits);
        auto joiner = CreateLLMCompilerJoiner(chat_model, options.joiner_input_parser, options.joiner_output_parser);
        return std::make_shared<LLMCompilerAgentExecutor>(stop_predicate, planer, worker, joiner, options);
    }


}

#endif //LLMCOMPILERAGENTEXECUTOR_HPP
