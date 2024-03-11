//
// Created by RobinQu on 2024/3/8.
//

#ifndef CHAIN_HPP
#define CHAIN_HPP

#include <llm.pb.h>
#include "functional/IRunnable.hpp"
#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    using ContextPtr = std::shared_ptr<LLMChainContext>;

    template<typename Output>
    class IChain : public IRunnable<LLMChainContext, Output> {

    };
}

#endif //CHAIN_HPP
