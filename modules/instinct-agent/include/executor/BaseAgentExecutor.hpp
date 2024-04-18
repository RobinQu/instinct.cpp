//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEAGENTEXECUTOR_HPP
#define BASEAGENTEXECUTOR_HPP

#include <utility>

#include "IAgentExecutor.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Base class for executor that handles state transitions of given agent.
     */
    class BaseAgentExecutor: public virtual IAgentExecutor, public BaseRunnable<AgentState, AgentState> {
        PlannerPtr planner_;
        WorkerPtr worker_;
        SolverPtr solver_;
        std::vector<FunctionToolSchema> all_schemas_;
    public:
        BaseAgentExecutor(PlannerPtr planner, WorkerPtr worker, SolverPtr solver)
            : planner_(std::move(planner)),
              worker_(std::move(worker)),
              solver_(std::move(solver)) {
            // do a copy for schema in toolkits
            for (const auto& tk: worker_->GetFunctionToolkits()) {
                for(const auto& schema: tk->GetAllFuncitonToolSchema()) {
                    all_schemas_.push_back(schema);
                }
            }
        }

        /**
         * Create `AgentState` with user prompt. This is a convenient method for Ad-hoc style execution. In a distributed agent server, `AgentState` should recover from ongoing request session.
         * @param input
         * @return Initial state
         */
        AgentState InitializeState(const PromptValueVariant& input) {
            const auto pv = MessageUtils::ConvertPromptValueVariantToPromptValue(input);
            AgentState agent_state;
            agent_state.mutable_input()->CopyFrom(pv);
            // TODO: avoid to copy schema for each invocation
            agent_state.mutable_function_tools()->Add(all_schemas_.begin(), all_schemas_.end());
            //for (auto& schema: all_schemas_) {
                // agent_state.mutable_function_tools()->Add()->CopyFrom(schema);
            //}
            return agent_state;
        }

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

        /**
         * Iterate all possible steps with given state. Agent may be finished or paused after execution.
         * @param agent_state
         * @return
         */
        AsyncIterator<AgentState> Stream(const AgentState& agent_state) override {
            return rpp::source::create<AgentState>([&](const auto& observer) {
                AgentState copied_state = agent_state;
                try {
                    AgentStep step;
                    do {
                        step = ResolveNextStep(copied_state);
                        LOG_DEBUG("Progressed step: {}", step.GetDescriptor()->name());
                        observer.on_next(copied_state);
                    } while (!step.has_finish());
                    LOG_DEBUG("Finished due to AgentFinish");
                    observer.on_completed();
                } catch (...) {
                    observer.on_error(std::current_exception());
                }
            });
        }

        [[nodiscard]] PlannerPtr GetPlaner() const override {
            return planner_;
        }

        [[nodiscard]] WorkerPtr GetWorker() const override {
            return worker_;
        }

        [[nodiscard]] SolverPtr GetSolver() const override {
            return solver_;
        }
    };


    using AgentExecutorPtr = std::shared_ptr<BaseAgentExecutor>;
}


#endif //BASEAGENTEXECUTOR_HPP
