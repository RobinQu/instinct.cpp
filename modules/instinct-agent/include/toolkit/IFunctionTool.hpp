//
// Created by RobinQu on 2024/4/8.
//

#ifndef IFUNCTIONTOOL_HPP
#define IFUNCTIONTOOL_HPP
#include "AgentGlobals.hpp"
#include <agent.pb.h>

#include "functional/StepFunctions.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_CORE_NS;
    class IFunctionTool {
    public:
        IFunctionTool()=default;
        virtual ~IFunctionTool()=default;
        IFunctionTool(const IFunctionTool&)=delete;
        IFunctionTool(IFunctionTool&&) = delete;
        [[nodiscard]] virtual const FunctionToolSchema& GetSchema() const = 0;
    };
}

#endif //IFUNCTIONTOOL_HPP
