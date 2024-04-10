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

        /**
         * Retrun the schema for tool rendering in function calling step
         * @return
         */
        [[nodiscard]] virtual const FunctionToolSchema& GetSchema() const = 0;

        /**
         * Define a simple self-test for function tool. Implementation should decide when and how the test is executed. Commna practice should be triggering self-check at program is started and report back.
         *
         * Self-test is meaningful for reasons:
         * 1. Underperformant model is configured for tool that cannot handle the job.
         * 2. Some tool has external dependencies which would fail time to time.
         * 3. Warm-up cache, model.
         * @return
         */
        virtual FunctionToolSelfCheckResponse SelfCheck() = 0;

    };
}

#endif //IFUNCTIONTOOL_HPP
