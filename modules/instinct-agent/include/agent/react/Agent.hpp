//
// Created by RobinQu on 2024/4/8.
//

#ifndef AGENT_HPP
#define AGENT_HPP


#include <agent.pb.h>
#include <sys/stat.h>

#include "AgentGlobals.hpp"
#include "LLMGlobals.hpp"
#include "ReACTAgentStateInputParser.hpp"
#include "ReACTAgentThoughtOutputParser.hpp"
#include "agent/BasePlanner.h"
#include "agent/BaseWorker.hpp"
#include "chain/LLMChain.hpp"
#include "chain/MessageChain.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "executor/BaseAgentExecutor.hpp"
#include "functional/StepFunctions.hpp"
#include "prompt/PlainChatPromptTemplate.hpp"
#include "functional/Xn.hpp"
#include "toolkit/BaseFunctionToolkit.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     * Create toolkits based worker that follows ReACT strategy
     */
    class ReACTWorker final: public BaseWorker {
    public:
        explicit ReACTWorker(const std::vector<FunctionToolkitPtr> &toolkits)
            : BaseWorker(toolkits) {
        }

        AgentObservationMessage Invoke(const AgentThoughtMessage &input) override {
            AgentObservationMessage observation_message;
            const auto& invocation = input.react().invocation();
            for(const auto& tk: GetFunctionToolkits()) {
                if (tk->LookupFunctionTool({.by_name = input.react().invocation().name()})) {
                    const auto ctx = CreateJSONContext();
                    ctx->ProduceMessage(invocation);
                    const auto fn_result = tk->Invoke(ctx)->RequireMessage<FunctionToolResult>();
                    observation_message.mutable_react()->CopyFrom(fn_result);
                    return observation_message;
                }
            }

            throw InstinctException(fmt::format("Unresolved invocation: id={}, name={}", invocation.id(), invocation.name()));
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
        Thought:{agent_scratchpad})"}
            });
        }

        return CreateFunctionalChain(
            CreateReACTAgentInputParser(),
            CreateReACTAgentThoughtOutputParser(),
            prompt_template | chat_model->AsModelFunction()
            );
    }


    class ReACTAgentExecutor final: public BaseAgentExecutor {
    public:
        ReACTAgentExecutor(const PlannerPtr &planner, const WorkerPtr &worker)
            : BaseAgentExecutor(planner, worker, nullptr) {
        }

        AgentStep ExecuteAgent(AgentState &state) override {
            AgentStep agent_step;
            const auto step_count = state.previous_steps_size();
            const auto& last_step = state.previous_steps(step_count-1);

            // run planner to think:
            // 1. if no previous steps exist
            // 2. if last step is observation
            if (state.previous_steps_size() == 0 || last_step.has_observation()) {
                const auto thought = GetPlaner()->Invoke(state);
                agent_step.mutable_thought()->CopyFrom(thought);
                return agent_step;
            }

            if (last_step.has_thought()) {
                const auto& thought = last_step.thought();
                if (thought.react().has_invocation()) {
                    // if last step is thought and contain invocation request, let's run function then.
                    const auto observation = GetWorker()->Invoke(thought);
                    agent_step.mutable_observation()->CopyFrom(observation);
                    return agent_step;
                }

                if (StringUtils::IsNotBlankString(thought.react().final_answer())) {
                    // no invocation but has non-blank final answer
                    agent_step.mutable_finish()->set_response(thought.react().final_answer());
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
            CreateReACTPlanner(chat_model, prompt_template),
            CreateReACTToolWorker(toolkits)
            );
    }


}


#endif //AGENT_HPP
