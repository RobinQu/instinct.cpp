//
// Created by RobinQu on 2024/5/11.
//

#ifndef LLMCOMPILERWORKER_HPP
#define LLMCOMPILERWORKER_HPP

#include "LLMGlobals.hpp"
#include "agent/BaseWorker.hpp"

namespace INSTINCT_LLM_NS {
    class LLMCompilerWorker final: public BaseWorker {
    public:
        AgentObservation Invoke(const AgentThought &input) override {

        }
    };
}


#endif //LLMCOMPILERWORKER_HPP
