//
// Created by RobinQu on 2024/5/8.
//

#ifndef LLMCOMPILERAGENTEXECUTOR_HPP
#define LLMCOMPILERAGENTEXECUTOR_HPP

#include "LLMGlobals.hpp"
#include "agent/executor/BaseAgentExecutor.hpp"
#include "agent/patterns/openai_tool/Agent.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerAgentExectuor final: public OpenAIToolAgentExecutor{
    public:
        LLMCompilerAgentExectuor(
            PlannerPtr planner,
            WorkerPtr worker,
            StopPredicate should_early_stop)
            : OpenAIToolAgentExecutor(std::move(planner), std::move(worker), std::move(should_early_stop)) {
        }
    };

    static AgentExecutorPtr CreateLLMCompilerAgentExecutor(const ChatModelPtr &chat_model,
        const std::vector<FunctionToolkitPtr> &toolkits,
        const OpenAIToolAgentExecutor::StopPredicate& stop_predicate = OpenAIToolAgentExecutor::NoStopPredicate) {

        return std::make_shared<LLMCompilerAgentExectuor>();

    }



}

#endif //LLMCOMPILERAGENTEXECUTOR_HPP
