//
// Created by RobinQu on 2024/4/8.
//

#ifndef IAGENTEXECUTOR_HPP
#define IAGENTEXECUTOR_HPP


#include "AgentGlobals.hpp"
#include "agent/BasePlanner.h"
#include "agent/BaseSolver.hpp"
#include "agent/BaseWorker.hpp"
#include "memory/BaseChatMemory.hpp"
#include "toolkit/BaseFunctionToolkit.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Interface class for agent executors. Implementation of this interface should expection following context protcol:
     * Context Input: PromptValue
     * Context Output: Generation
     */
    class IAgentExecutor {
    public:
        IAgentExecutor(IAgentExecutor&&)=delete;
        IAgentExecutor(const IAgentExecutor&)=delete;
        virtual ~IAgentExecutor() = default;
        IAgentExecutor()=default;


        [[nodiscard]] virtual PlannerPtr GetPlaner() const = 0;

        [[nodiscard]] virtual WorkerPtr GetWorker() const = 0;

        [[nodiscard]] virtual SolverPtr GetSolver() const = 0;

    };
}

#endif //IAGENTEXECUTOR_HPP
