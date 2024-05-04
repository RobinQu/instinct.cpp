//
// Created by RobinQu on 2024/4/28.
//

#ifndef OPENAITOOLAGENTEXECUTOR_HPP
#define OPENAITOOLAGENTEXECUTOR_HPP

#include "LLMGlobals.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * agent executor that relies on a model with built-in tool calling API
     */
    class OpenAIToolAgentExecutor final : public BaseAgentExecutor {

        /**
         * Tool executor that supports parallel tool calling using thread pool
         */
        class OpenAIToolAgentWorker final : public BaseWorker {
            ThreadPool thread_pool_;

        public:
            explicit OpenAIToolAgentWorker(const std::vector<FunctionToolkitPtr> &toolkits)
                : BaseWorker(toolkits) {
            }

            AgentObservation Invoke(const AgentThought &input) override {
                const auto &tool_request_msg = input.continuation().openai().tool_call_message();

                std::vector<ToolCallObject> filtered_tool_calls;
                for (const auto& call: tool_request_msg.tool_calls()) {
                    for (const auto &tk: GetFunctionToolkits()) {
                        if (tk->LookupFunctionTool({.by_name = call.function().name()})) {
                            filtered_tool_calls.push_back(call);
                        }
                    }
                }

                // only execute tool call that has matching tools in worker
                auto multi_futures = thread_pool_.submit_sequence(0, tool_request_msg.tool_calls_size(), [&](auto i) {
                    const auto &call = filtered_tool_calls.at(i);
                    FunctionToolInvocation invocation;
                    invocation.set_id(call.id());
                    invocation.set_name(call.function().name());
                    invocation.set_input(call.function().arguments());
                    for (const auto &tk: GetFunctionToolkits()) {
                        if (tk->LookupFunctionTool({.by_name = call.function().name()})) {
                            return tk->Invoke(invocation);
                        }
                    }
                    // impossible to reach here
                    throw InstinctException(fmt::format("Unresolved invocation: id={}, name={}", invocation.id(),
                                                        invocation.name()));
                });

                AgentObservation observation_message;
                auto *observation = observation_message.mutable_openai();;
                for (auto &future: multi_futures) {
                    const auto tool_result = future.get();
                    if (tool_result.has_error()) {
                        LOG_ERROR("invocation failed: id={}, exception={}", tool_result.invocation_id(), tool_result.exception());
                        throw InstinctException(tool_result.exception());
                    }
                    Message function_message;
                    function_message.set_role("tool");
                    function_message.set_tool_call_id(tool_result.invocation_id());
                    function_message.set_content(tool_result.return_value());
                    observation->add_tool_messages()->CopyFrom(function_message);
                }
                return observation_message;
            }
        };

        static WorkerPtr CreateOpenAIToolAgentWorker(const std::vector<FunctionToolkitPtr> &toolkits) {
            return std::make_shared<OpenAIToolAgentWorker>(toolkits);
        }

        class OpenAIToolAgentPlanner final: public BaseRunnable<AgentState, AgentThought> {
            ChatModelPtr chat_model_;
        public:
            explicit OpenAIToolAgentPlanner(ChatModelPtr chatModel) : chat_model_(std::move(chatModel)) {}

            AgentThought Invoke(const AgentState &state) override {
                chat_model_->BindToolSchemas({state.function_tools().begin(), state.function_tools().end()});

                // this should contain the first message from user
                PromptValue prompt_value = state.input();
                for (const auto &step: state.previous_steps()) {
                    // add tool call message ChatCompletion api
                    if (step.has_thought()
                        && step.thought().has_continuation()
                        && step.thought().continuation().has_openai()
                        && step.thought().continuation().openai().has_tool_call_message()) {
                        prompt_value.mutable_chat()->add_messages()->CopyFrom(
                                step.thought().continuation().openai().tool_call_message()
                        );
                    }
                    if (step.has_observation() && step.observation().has_openai() && step.observation().openai().
                            tool_messages_size() > 0) {
                        // add tool call results
                        for (const auto &msg: step.observation().openai().tool_messages()) {
                            prompt_value.mutable_chat()->add_messages()->CopyFrom(msg);
                        }
                    }
                }

                const auto message = chat_model_->Invoke(prompt_value);
                AgentThought thought_message;
                if (message.tool_calls_size() > 0) { // has more tool call requests
                    thought_message.mutable_continuation()->mutable_openai()->mutable_tool_call_message()->CopyFrom(message);
                    return thought_message;
                }
                thought_message.mutable_finish()->set_response(message.content());
                return thought_message;
            }
        };

        /**
         * Create OpenAI style tool agent
         * @param chat_model A chat model provider that supports tool calling APIs
         * @return
         */
        static PlannerPtr CreateOpenAIToolAgentPlanner(const ChatModelPtr &chat_model) {
            return std::make_shared<OpenAIToolAgentPlanner>(chat_model);
        }


    public:
        /**
         * Functional callback, which is invoked after a step is generated from agent execuctor.
         * Retrun false to terminate execution loop.
         *
         */
        using StopPredicate = std::function<bool(const AgentState&, AgentStep&)>;


        /**
         * Default predicate for OpenAIToolAgentExecutor. It won't stop agent from resolving next step.
         * @return
         */
        static bool NoStopPredicate(const AgentState&, AgentStep&) {
            return false;
        }

        OpenAIToolAgentExecutor(const PlannerPtr &planner, const WorkerPtr &worker, StopPredicate should_early_stop = NoStopPredicate)
            : BaseAgentExecutor(planner, worker), should_early_stop_(std::move(should_early_stop)) {
        }

        OpenAIToolAgentExecutor(const ChatModelPtr &chat_model,
                                const std::vector<FunctionToolkitPtr> &toolkits,
                                StopPredicate should_early_stop = NoStopPredicate): BaseAgentExecutor(
            CreateOpenAIToolAgentPlanner(chat_model),
            CreateOpenAIToolAgentWorker(toolkits)),
            should_early_stop_(std::move(should_early_stop)
        ) {
            for(const auto& tk: toolkits) {
                chat_model->BindTools(tk);
            }
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            AgentStep agent_step;
            // check if early stop is required
            if (!should_early_stop_(state, agent_step)) {
                return agent_step;
            }

            // while not is_last:
            //  do ChatCompletion with messages -> message
            //  is_last = messages.function_call is None
            const auto n = state.previous_steps_size();
            const auto last_step = state.previous_steps(n - 1);
            if (n == 0 || last_step.has_observation()) {
                // do planing
                AgentThought thought_step = GetPlaner()->Invoke(state);
                agent_step.mutable_thought()->CopyFrom(thought_step);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            if (last_step.has_thought()
                && last_step.thought().has_pause()
                && last_step.thought().pause().has_openai()
                && last_step.thought().pause().openai().has_tool_call_message()
                    ) {
                // for pause step, user should submit rest of tool results through IRunService::SubmitToolOutputs.
                // so we just check if all tools are done here
                const auto& pause = last_step.thought().pause().openai();
                if (pause.tool_call_message().tool_calls_size() == pause.completed_size()) {
                    // lift to observation
                    OpenAIToolAgentObservation observation_message;
                    observation_message.mutable_tool_messages()->CopyFrom(pause.completed());
                    agent_step.mutable_observation()->CopyFrom(observation_message);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }
            }

            // if last step is paused or finished, it cannot be executed again
            if (last_step.has_thought()
                    && last_step.thought().has_continuation()
                    && last_step.thought().continuation().has_openai()
                    && last_step.thought().continuation().openai().has_tool_call_message()
                    && last_step.thought().continuation().openai().tool_call_message().tool_calls_size() > 0
                    ) {
                const auto& tool_call_message = last_step.thought().continuation().openai().tool_call_message();
                const auto& tool_call_objects = tool_call_message.tool_calls();
                const auto& thought_step = last_step.thought();

                // worker should filter out unsupported tool
                const auto observation_message = GetWorker()->Invoke(thought_step);
                int completed = 0;
                for(const auto& tool_call: tool_call_objects) {
                    for(const auto& tool_message: observation_message.openai().tool_messages()) {
                        if (tool_message.tool_call_id() == tool_call.id()) {
                            completed++;
                        }
                    }
                }

                if (completed == tool_call_objects.size()) { // return a pause step
                    auto* pause = agent_step.mutable_thought()->mutable_pause()->mutable_openai();
                    pause->mutable_tool_call_message()->CopyFrom(tool_call_message);
                    pause->mutable_completed()->CopyFrom(observation_message.openai().tool_messages());
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
    };

    static AgentExecutorPtr CreateOpenAIToolAgentExecutor(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits,
        const OpenAIToolAgentExecutor::StopPredicate& stop_predicate = OpenAIToolAgentExecutor::NoStopPredicate) {
        return std::make_shared<OpenAIToolAgentExecutor>(chat_model, toolkits, stop_predicate);
    }
}


#endif //OPENAITOOLAGENTEXECUTOR_HPP
