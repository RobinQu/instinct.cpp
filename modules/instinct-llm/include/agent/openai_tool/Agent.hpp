//
// Created by RobinQu on 2024/4/28.
//

#ifndef OPENAITOOLAGENTEXECUTOR_HPP
#define OPENAITOOLAGENTEXECUTOR_HPP

#include "LLMGlobals.hpp"
#include "agent_executor/BaseAgentExecutor.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"

namespace
INSTINCT_LLM_NS {
    /**
     * agent executor that relies on a model with built-in tool calling API
     */
    class OpenAIToolAgentExecutor final : public BaseAgentExecutor {

        /**
         * Tool executor that supports paralled tool calling using thread pool
         */
        class OpenAIToolAgentWorker final : public BaseWorker {
            ThreadPool thread_pool_;

        public:
            explicit OpenAIToolAgentWorker(const std::vector<FunctionToolkitPtr> &toolkits)
                : BaseWorker(toolkits) {
            }

            AgentObservationMessage Invoke(const AgentThoughtMessage &input) override {
                const auto &tool_request_msg = input.openai().tool_call_message();
                auto multi_futures = thread_pool_.submit_sequence(0, tool_request_msg.tool_calls_size(), [&](auto i) {
                    const auto &call = tool_request_msg.tool_calls(i);
                    FunctionToolInvocation invocation;
                    invocation.set_id(call.id());
                    invocation.set_name(call.function().name());
                    invocation.set_input(call.function().arguments());
                    for (const auto &tk: GetFunctionToolkits()) {
                        if (tk->LookupFunctionTool({.by_name = call.function().name()})) {
                            return tk->Invoke(invocation);
                        }
                    }
                    throw InstinctException(fmt::format("Unresolved invocation: id={}, name={}", invocation.id(),
                                                        invocation.name()));
                });

                AgentObservationMessage observation_message;
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

        class OpenAIToolAgentStateInputParser final: public BaseInputParser<AgentState> {
        public:
            JSONContextPtr ParseInput(const AgentState &state) override {
                JSONContextPtr ctx = CreateJSONContext();

                // this should contain the first mesage from user
                PromptValue prompt_value = state.input();
                for (const auto &step: state.previous_steps()) {
                    // add tool call message ChatCompletion api
                    if (step.has_thought() && step.thought().has_openai() && step.thought().openai().
                        has_tool_call_message()) {
                        prompt_value.mutable_chat()->add_messages()->CopyFrom(
                            step.thought().openai().tool_call_message());
                    }
                    if (step.has_observation() && step.observation().has_openai() && step.observation().openai().
                        tool_messages_size() > 0) {
                        // add tool call results
                        for (const auto &msg: step.observation().openai().tool_messages()) {
                            prompt_value.mutable_chat()->add_messages()->CopyFrom(msg);
                        }
                    }
                }
                ctx->ProduceMessage(prompt_value);
                return ctx;
            }
        };

        class OpenAIToolAgentThoughtOutputParser final : public BaseOutputParser<AgentThoughtMessage> {
        public:
            AgentThoughtMessage ParseResult(const Generation &context) override {
                AgentThoughtMessage thought_message;
                thought_message.mutable_openai()->mutable_tool_call_message()->CopyFrom(context.message());
                return thought_message;
            }
        };

        static PlannerPtr CreateOpenAIToolAgentPlanner(const ChatModelPtr &chat_model) {
            const InputParserPtr<AgentState> input_parser = std::make_shared<OpenAIToolAgentStateInputParser>();
            const OutputParserPtr<AgentThoughtMessage> output_parser = std::make_shared<OpenAIToolAgentThoughtOutputParser>();
            return CreateFunctionalChain(
                input_parser,
                output_parser,
                chat_model->AsModelFunction()
            );
        }

    public:
        OpenAIToolAgentExecutor(const PlannerPtr &planner, const WorkerPtr &worker)
            : BaseAgentExecutor(planner, worker) {
        }

        OpenAIToolAgentExecutor(const ChatModelPtr &chat_model,
                                const std::vector<FunctionToolkitPtr> &toolkits): BaseAgentExecutor(
            CreateOpenAIToolAgentPlanner(chat_model),
            CreateOpenAIToolAgentWorker(toolkits)
        ) {
            for(const auto& tk: toolkits) {
                chat_model->BindTools(tk);
            }
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            AgentStep agent_step;
            // while not is_last:
            //  do ChatCompletion with messages -> message
            //  is_last = messages.function_call is None
            const auto n = state.previous_steps_size();
            if (n == 0 || state.previous_steps(n - 1).has_observation()) {
                // do planing
                const AgentThoughtMessage thought_step = GetPlaner()->Invoke(state);
                assert_true(thought_step.has_openai());
                agent_step.mutable_thought()->CopyFrom(thought_step);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            if (state.previous_steps(n - 1).has_thought()) {
                const AgentThoughtMessage thought_step = state.previous_steps(n - 1).thought();
                assert_true(thought_step.has_openai());

                if (thought_step.openai().tool_call_message().tool_calls_size() > 0) {
                    const auto observation_message = GetWorker()->Invoke(thought_step);
                    agent_step.mutable_observation()->CopyFrom(observation_message);
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }
                // if no tool calls are requested, then finalize
                AgentFinishStepMessage finish_step_message;
                finish_step_message.set_response(thought_step.openai().tool_call_message().content());
                agent_step.mutable_finish()->CopyFrom(finish_step_message);
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            LOG_DEBUG("illegal state: {}", state.ShortDebugString());
            throw InstinctException("IllegalState for OpenAIToolAgentExeuctor");
        }
    };

    static AgentExecutorPtr CreateOpenAIToolAgentExecutor(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits) {
        return std::make_shared<OpenAIToolAgentExecutor>(chat_model, toolkits);
    }
}


#endif //OPENAITOOLAGENTEXECUTOR_HPP
