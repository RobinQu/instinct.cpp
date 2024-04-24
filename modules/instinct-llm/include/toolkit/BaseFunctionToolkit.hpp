//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEFUNCTIONTOOLKIT_HPP
#define BASEFUNCTIONTOOLKIT_HPP
#include "IFunctionToolkit.hpp"

namespace INSTINCT_LLM_NS {
    class BaseFunctionToolkit: public virtual IFunctionToolKit, public BaseRunnable<FunctionToolInvocation, FunctionToolResult> {
    public:
        FunctionToolResult Invoke(const FunctionToolInvocation &invocation) override {
            const auto& fn_name = invocation.name();
            if (const auto fn_tool = this->LookupFunctionTool({.by_name = fn_name})) {
                return fn_tool->Invoke(invocation);
            }
            FunctionToolResult failed_result;
            failed_result.set_exception("Failed to lookup function tool in this toolkit. Target function name: " + fn_name);
            failed_result.set_invocation_id(invocation.id());
            return failed_result;
        }
    };

    using FunctionToolkitPtr = std::shared_ptr<BaseFunctionToolkit>;
}





#endif //BASEFUNCTIONTOOLKIT_HPP
