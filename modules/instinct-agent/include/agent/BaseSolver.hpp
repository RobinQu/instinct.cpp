//
// Created by RobinQu on 2024/4/9.
//

#ifndef BASESOLVER_HPP
#define BASESOLVER_HPP


#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {
    class BaseSolver : public BaseRunnable<AgentState, AgentStep> {

    };
    using SolverPtr = std::shared_ptr<BaseSolver>;
}

#endif //BASESOLVER_HPP
