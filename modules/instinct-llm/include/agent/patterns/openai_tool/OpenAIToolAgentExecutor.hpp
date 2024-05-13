//
// Created by RobinQu on 2024/4/28.
//

#ifndef OPENAITOOLAGENTEXECUTOR_HPP
#define OPENAITOOLAGENTEXECUTOR_HPP

#include "LLMGlobals.hpp"
#include "OpenAIToolAgentPlanner.hpp"
#include "OpenAIToolAgentWorker.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * agent executor that relies on a model with built-in tool calling API
     */
    class OpenAIToolAgentExecutor final: public BaseAgentExecutor {

    public:

        OpenAIToolAgentExecutor(PlannerPtr planner, WorkerPtr worker, StopPredicate should_early_stop = NoStopPredicate):
            should_early_stop_(std::move(should_early_stop)),
            planner_(std::move(planner)),
            worker_(std::move(worker)) {
        }

        OpenAIToolAgentExecutor(const ChatModelPtr &chat_model,
                                const std::vector<FunctionToolkitPtr> &toolkits,
                                StopPredicate should_early_stop = NoStopPredicate):
            should_early_stop_(std::move(should_early_stop)),
            planner_(CreateOpenAIToolAgentPlanner(chat_model)),
            worker_(CreateOpenAIToolAgentWorker(toolkits))
        {
            for(const auto& tk: toolkits) {
                chat_model->BindTools(tk);
            }
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            AgentStep agent_step;
            // check if early stop is required
            if (should_early_stop_(state, agent_step)) {
                return agent_step;
            }

            // while not is_last:
            //  do ChatCompletion with messages -> message
            //  is_last = messages.function_call is None
            const auto n = state.previous_steps_size();
            if (n == 0 || state.previous_steps(n - 1).has_observation()) {
                // do planing
                const AgentThought thought_step = planner_->Invoke(state);
                agent_step.mutable_thought()->CopyFrom(thought_step);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            const auto& last_step = state.previous_steps(n - 1);
            if (last_step.has_thought()
                && last_step.thought().has_pause()
                && last_step.thought().pause().has_tool_call_message()
                    ) {
                // for pause step, user should submit rest of tool results through IRunService::SubmitToolOutputs.
                // so we just check if all tools are done here
                const auto& pause = last_step.thought().pause();
                if (pause.tool_call_message().tool_calls_size() == pause.completed_size()) {
                    // lift to observation
                    agent_step.mutable_observation()->mutable_tool_messages()->CopyFrom(pause.completed());
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }
                const auto call_ids_view = pause.tool_call_message().tool_calls() | std::views::transform([](const ToolCallObject& tool_call) {
                    return tool_call.id();
                });
                LOG_WARN("Some tool calls are not finished. Expected {}, Finished {}, Tool call ids: {}", pause.tool_call_message().tool_calls_size(), pause.completed_size(), StringUtils::JoinWith(call_ids_view, ","));
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

                // worker should filter out unsupported tool
                const auto observation_message = worker_->Invoke(thought_step);
                int completed = 0;
                for(const auto& tool_call: tool_call_objects) {
                    for(const auto& tool_message: observation_message.tool_messages()) {
                        if (tool_message.tool_call_id() == tool_call.id()) {
                            completed++;
                        }
                    }
                }

                if (completed != tool_call_objects.size()) { // return a pause step
                    auto* pause = agent_step.mutable_thought()->mutable_pause();
                    pause->mutable_tool_call_message()->CopyFrom(tool_call_message);
                    pause->mutable_completed()->CopyFrom(observation_message.tool_messages());
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }

                agent_step.mutable_observation()->CopyFrom(observation_message);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            LOG_DEBUG("illegal state: {}", state.ShortDebugString());
            throw InstinctException("IllegalState for OpenAIToolAgentExecutor");
        }

    private:
        StopPredicate should_early_stop_;
        PlannerPtr planner_;
        WorkerPtr worker_;
    };

    static AgentExecutorPtr CreateOpenAIToolAgentExecutor(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits,
        const StopPredicate& stop_predicate = NoStopPredicate) {
        return std::make_shared<OpenAIToolAgentExecutor>(chat_model, toolkits, stop_predicate);
    }
}


#endif //OPENAITOOLAGENTEXECUTOR_HPP
