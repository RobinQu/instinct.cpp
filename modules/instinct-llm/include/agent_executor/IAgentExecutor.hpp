//
// Created by RobinQu on 2024/4/8.
//

#ifndef IAGENTEXECUTOR_HPP
#define IAGENTEXECUTOR_HPP


#include "LLMGlobals.hpp"
#include "agent/BaseSolver.hpp"
#include "agent/BaseWorker.hpp"
#include "chain/MessageChain.hpp"

namespace INSTINCT_LLM_NS {

    using Planer = MessageChain<AgentState, AgentThoughtMessage>;
    using PlannerPtr = MessageChainPtr<AgentState, AgentThoughtMessage>;


    /**
     * Interface class for agent executors.
     */
    class IAgentExecutor {
    public:
        IAgentExecutor(IAgentExecutor&&)=delete;
        IAgentExecutor(const IAgentExecutor&)=delete;
        virtual ~IAgentExecutor() = default;
        IAgentExecutor()=default;

        [[nodiscard]] virtual PlannerPtr GetPlaner() const = 0;

        [[nodiscard]] virtual WorkerPtr GetWorker() const = 0;

        // [[nodiscard]] virtual SolverPtr GetSolver() const = 0;

        /**
         * Generate and execute next step.
         * @param state the current agent state. State will be modified afterwards.
         * @return completed step
         */
        virtual AgentStep ResolveNextStep(AgentState& state) = 0;

        // virtual void Resume();
    };
}

#endif //IAGENTEXECUTOR_HPP
