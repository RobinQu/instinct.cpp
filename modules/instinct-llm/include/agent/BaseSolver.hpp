//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESOLVER_HPP
#define BASESOLVER_HPP


#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    class BaseSolver : public BaseRunnable<AgentState, AgentStep> {

    };
    using SolverPtr = std::shared_ptr<BaseSolver>;
}

#endif //BASESOLVER_HPP
