//
// Created by RobinQu on 2024/4/8.
//

#ifndef IAGENTEXECUTOR_HPP
#define IAGENTEXECUTOR_HPP


#include <instinct/llm_global.hpp>
#include <instinct/chain/message_chain.hpp>

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
            // const auto pv = MessageUtils::ConvertPromptValueVariantToPromptValue(input);
            const auto content = details::conv_prompt_value_variant_to_string(input);
            AgentState agent_state;
            auto* user_message = agent_state.mutable_input()->mutable_chat()->add_messages();
            user_message->set_content(content);
            user_message->set_role("user");
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

#endif //IAGENTEXECUTOR_HPP
