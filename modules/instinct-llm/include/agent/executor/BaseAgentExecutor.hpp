//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEAGENTEXECUTOR_HPP
#define BASEAGENTEXECUTOR_HPP


#include "IAgentExecutor.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * Base class for executor that handles state transitions of given agent.
     */
    class BaseAgentExecutor: public virtual IAgentExecutor, public BaseRunnable<AgentState, AgentState> {
    public:


        /**
         * Return the final step for given state
         * @param agent_state
         * @return
         */
        AgentState Invoke(const AgentState& agent_state) override {
            AgentState state;
            Stream(agent_state)
                | rpp::operators::as_blocking()
                | rpp::operators::last()
                | rpp::operators::subscribe([&](const auto& final_state) {
                    state = final_state;
                });
            return state;
        }

        virtual AgentState InitializeState(const PromptValueVariant& input) {
            const auto pv = MessageUtils::ConvertPromptValueVariantToPromptValue(input);
            AgentState agent_state;
            agent_state.mutable_input()->CopyFrom(pv);
            return agent_state;
        }

        /**
         * Iterate all possible steps with given state. Agent may be finished or paused after execution.
         * @param agent_state
         * @return
         */
        AsyncIterator<AgentState> Stream(const AgentState& agent_state) override {
            return rpp::source::create<AgentState>([&](const auto& observer) {
                AgentState copied_state = agent_state;
                try {
                    AgentStep step = ResolveNextStep(copied_state);
                    observer.on_next(copied_state);
                    while (
                            step.has_observation() ||
                            (step.has_thought() && step.thought().has_continuation())
                    ) {
                        step = ResolveNextStep(copied_state);
                        observer.on_next(copied_state);
                    }
                    observer.on_completed();
                } catch (...) {
                    observer.on_error(std::current_exception());
                }
            });
        }


    };


    using AgentExecutorPtr = std::shared_ptr<BaseAgentExecutor>;
}


#endif //BASEAGENTEXECUTOR_HPP
