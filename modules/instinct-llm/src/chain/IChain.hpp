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
    template<typename Output>
    // requires is_pb_message<std::remove_pointer_t<Output>>
    class IChain : public IRunnable<LLMChainContext, Output> {
        // TODO schema for input validation
        // virtual std::vector<VariableID> GetInputVariableIDs() = 0;
    };
}

#endif //CHAIN_HPP
