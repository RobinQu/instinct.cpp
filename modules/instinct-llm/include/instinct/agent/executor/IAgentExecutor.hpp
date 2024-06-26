//
// Created by RobinQu on 2024/4/8.
//

#ifndef IAGENTEXECUTOR_HPP
#define IAGENTEXECUTOR_HPP


#include "../../LLMGlobals.hpp"
#include "../../chain/MessageChain.hpp"

namespace INSTINCT_LLM_NS {

    using Planer = BaseRunnable<AgentState, AgentThought>;
    using PlannerPtr = RunnablePtr<AgentState, AgentThought>;


    /**
     * Interface class for agent executors.
     */
    class IAgentExecutor {
    public:
        IAgentExecutor(IAgentExecutor&&)=delete;
        IAgentExecutor(const IAgentExecutor&)=delete;
        virtual ~IAgentExecutor() = default;
        IAgentExecutor()=default;

        /**
         * Generate and execute next step.
         * @param state the current agent state. State will be modified afterwards.
         * @return completed step
         */
        virtual AgentStep ResolveNextStep(AgentState& state) = 0;
    };
}

#endif //IAGENTEXECUTOR_HPP
