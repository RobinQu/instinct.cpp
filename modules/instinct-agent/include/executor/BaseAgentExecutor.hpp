//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEAGENTEXECUTOR_HPP
#define BASEAGENTEXECUTOR_HPP

#include <utility>

#include "IAgentExecutor.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Base class for executor that handles the actual execution flow
     * Context Input: PromptValue
     * Context Output: AgentStep
     */
    class BaseAgentExecutor: public virtual IAgentExecutor, public BaseStepFunction {
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

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            JSONContextPtr result;
            Stream(input)
                | rpp::operators::as_blocking()
                | rpp::operators::last()
                | rpp::operators::subscribe([&](const auto& ctx) {
                    result = ctx;
                });
            return result;
        }

        AsyncIterator<JSONContextPtr> Stream(
            const JSONContextPtr &input) override {
            const auto pv = input->RequireMessage<PromptValue>();
            return rpp::source::create<JSONContextPtr>([&, pv](const auto& observer) {
                AgentState agent_state;
                agent_state.mutable_input()->CopyFrom(pv);
                // avoid to copy schema for each invocation
                for (auto& schema: all_schemas_) {
                    agent_state.mutable_function_tools()->AddAllocated(&schema);
                }

                try {
                    const auto step = ExecuteAgent(agent_state);
                    auto result = CreateJSONContext();
                    result->ProduceMessage(step);
                    observer.on_next(result);
                    if (step.has_finish()) {
                        observer.on_completed();
                    }
                } catch (...) {
                    observer.on_error(std::current_exception());
                }
            });
        }


        /**
         * Concrete method to perform action with given state and return next step
         * @param state
         * @return
         */
        virtual AgentStep ExecuteAgent(AgentState& state) = 0;

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
