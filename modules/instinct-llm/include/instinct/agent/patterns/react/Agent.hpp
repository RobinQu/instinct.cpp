//
// Created by RobinQu on 2024/4/8.
//

#ifndef AGENT_HPP
#define AGENT_HPP


#include <agent.pb.h>

#include <instinct/LLMGlobals.hpp>
#include <instinct/agent/patterns/react/ReACTAgentStateInputParser.hpp>
#include <instinct/agent/patterns/react/ReACTAgentThoughtOutputParser.hpp>
#include <instinct/agent/BaseWorker.hpp>
#include <instinct/chain/LLMChain.hpp>
#include <instinct/chain/MessageChain.hpp>
#include <instinct/chat_model/BaseChatModel.hpp>
#include <instinct/agent/executor/BaseAgentExecutor.hpp>
#include <instinct/functional/step_functions.hpp>
#include <instinct/prompt/PlainChatPromptTemplate.hpp>
#include <instinct/functional/xn.hpp>
#include <instinct/toolkit/BaseFunctionToolkit.hpp>

namespace INSTINCT_LLM_NS {

    /**
     * Create toolkits based worker that follows ReACT strategy
     */
    class ReACTWorker final: public BaseWorker {
    public:
        explicit ReACTWorker(const std::vector<FunctionToolkitPtr> &toolkits)
            : BaseWorker(toolkits) {
        }

        AgentObservation Invoke(const AgentThought &input) override {
            AgentObservation observation_message;
            assert_true(input.has_continuation() && input.continuation().has_tool_call_message() && input.continuation().tool_call_message().tool_calls_size()>0, "should have continuation for ReACT executor.");
            // only use first tool call
            const auto& invocation = input.continuation().tool_call_message().tool_calls(0);
            for(const auto& tk: GetFunctionToolkits()) {
                if (tk->LookupFunctionTool({.by_name = invocation.function().name()})) {
                    const auto fn_result = tk->Invoke(invocation);
                    if (fn_result.has_error()) {
                        // TODO more delicate handling
                        throw InstinctException(fmt::format("Function execution failed. invocation: {}, exception: {}", invocation.ShortDebugString(), fn_result.exception()));
                    }
                    auto *tool_message = observation_message.mutable_tool_messages()->Add();
                    tool_message->set_content(fn_result.return_value());
                    tool_message->set_tool_call_id(fn_result.invocation_id());
                    return observation_message;
                }
            }

            throw InstinctException(fmt::format("Unresolved invocation: id={}, name={}", invocation.id(), invocation.function().name()));
        }

    };

    static WorkerPtr CreateReACTToolWorker(const std::vector<FunctionToolkitPtr> &toolkits) {
        return std::make_shared<ReACTWorker>(toolkits);
    }

    /**
     * Simple planner that uses ReACT strategy to solve a specific problem
     */
    static PlannerPtr CreateReACTPlanner(
        const ChatModelPtr &chat_model,
        PromptTemplatePtr prompt_template = nullptr
    ) {
        if (!prompt_template) {
            prompt_template = CreatePlainChatPromptTemplate({
                {kSystem,  R"(Answer the following questions as best you can. You have access to the following tools:

{tools}

Use the following format:

Question: the input question you must answer
Thought: you should always think about what to do
Action: the action to take, should be one of [{tool_names}]
Action Input: the input to the action, which is formatted as JSON blob with 'name' and 'arguments' keys.
Observation: the result of the action
... (this Thought/Action/Action Input/Observation can repeat N times)
Thought: I now know the final answer
Final Answer: the final answer to the original input question

Begin!

Question: {input}
Thought: {agent_scratchpad})"
                }});
        }

        return CreateFunctionalChain(
            CreateReACTAgentInputParser(),
            CreateReACTAgentThoughtOutputParser(),
            prompt_template | chat_model->AsModelFunction()
            );
    }

    struct ReACTAgentExecutorOptions {
        u_int8_t max_react_loop = 6;
    };

    /**
     * AgentExecutor that follows ReACT strategy. Let's think, act and observe.
     * In the exeuction flow, planer will generate a thought, while worker will handle invocation according to action input and give feedback to LLM as observation.
     * It will loop these procedures until final answer is generated or max loop count is reached.
     */
    class ReACTAgentExecutor final: public BaseAgentExecutor {
        PlannerPtr planner_;
        WorkerPtr worker_;
        ChatModelPtr chat_model_;
        std::vector<FunctionTool> all_schemas_;
    public:
        ReACTAgentExecutor(
            const ChatModelPtr &chat_model,
            const std::vector<FunctionToolkitPtr> &toolkits,
            const PromptTemplatePtr& prompt_template = nullptr
        ):
            planner_(CreateReACTPlanner(chat_model, prompt_template)),
            worker_(CreateReACTToolWorker(toolkits)),
            chat_model_(chat_model)
        {
            for (const auto& tk: toolkits) {
                all_schemas_.insert(all_schemas_.end(), tk->GetAllFunctionToolSchema().begin(), tk->GetAllFunctionToolSchema().end());
            }
        }

        /**
         * Create `AgentState` with user prompt. This is a convenient method for Ad-hoc style execution. In a distributed agent server, `AgentState` should recover from ongoing request session.
         * @param input
         * @return Initial state
         */
        AgentState InitializeState(const PromptValueVariant& input) override {
            auto agent_state = BaseAgentExecutor::InitializeState(input);
            agent_state.mutable_function_tools()->Add(all_schemas_.begin(), all_schemas_.end());
            return agent_state;
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            AgentStep agent_step;
            const auto step_count = state.previous_steps_size();

            // run planner to think:
            // 1. if no previous steps exist
            // 2. if last step is observation
            if (step_count == 0 || state.previous_steps(step_count-1).has_observation()) {
                const auto thought = planner_->Invoke(state);
                LOG_DEBUG("ReACT thought: {}", thought.ShortDebugString());
                agent_step.mutable_thought()->CopyFrom(thought);
                // save this step
                state.add_previous_steps()->CopyFrom(agent_step);
                return agent_step;
            }

            if (const auto& last_step = state.previous_steps(step_count-1); last_step.has_thought()) {
                const auto& thought = last_step.thought();

                assert_true(thought.has_continuation(), "thought should have continuation");

                if (thought.has_continuation() && thought.continuation().has_tool_call_message()) {
                    // if last step is thought and contain invocation request, let's run function then.
                    const auto observation = worker_->Invoke(thought);
                    LOG_DEBUG("ReACT observation: {}", observation.ShortDebugString());
                    agent_step.mutable_observation()->CopyFrom(observation);
                    // save this step
                    state.add_previous_steps()->CopyFrom(agent_step);
                    return agent_step;
                }
            }

            LOG_DEBUG("illegal state: {}", state.ShortDebugString());
            throw InstinctException("IllegalState for ReACTAgentExecutor");
        }
    };


    static AgentExecutorPtr CreateReACTAgentExecutor(
        const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits,
        const PromptTemplatePtr& prompt_template = nullptr
        ) {
        return std::make_shared<ReACTAgentExecutor>(
            chat_model,
            toolkits,
            prompt_template
            );
    }


}


#endif //AGENT_HPP
