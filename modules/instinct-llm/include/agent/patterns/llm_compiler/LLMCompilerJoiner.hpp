//
// Created by RobinQu on 2024/5/11.
//

#ifndef LLMCOMPILERJOINER_HPP
#define LLMCOMPILERJOINER_HPP

#include "LLMGlobals.hpp"
#include "chain/MessageChain.hpp"

namespace INSTINCT_LLM_NS {
    class LLMcompilerJoiner final: public MessageChain<AgentState, AgentThought> {

    };

    using JoinerPtr = std::shared_ptr<LLMcompilerJoiner>;

}

#endif //LLMCOMPILERJOINER_HPP
