//
// Created by RobinQu on 2024/4/28.
//

#ifndef OPENAITOOLAGENTEXECUTOR_HPP
#define OPENAITOOLAGENTEXECUTOR_HPP

#include "LLMGlobals.hpp"
#include "agent_executor/BaseAgentExecutor.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * agent executor that relies on a model with built-in tool calling API
     */
    class OpenAIToolAgentExeuctor: public BaseAgentExecutor {
    public:
        OpenAIToolAgentExeuctor(const PlannerPtr &planner, const WorkerPtr &worker, const SolverPtr &solver)
            : BaseAgentExecutor(planner, worker, solver) {
        }

        AgentStep ResolveNextStep(AgentState &state) override {
            // while not is_last:
            //  do ChatCompletion with messages -> message
            //  is_last = messages.function_call is None

            const auto thought_step = GetPlaner()->Invoke(state);


            AgentStep agent_step;
            return agent_step;
        }
    };
}


#endif //OPENAITOOLAGENTEXECUTOR_HPP
