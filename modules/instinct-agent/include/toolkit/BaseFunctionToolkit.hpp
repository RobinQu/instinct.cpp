//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEFUNCTIONTOOLKIT_HPP
#define BASEFUNCTIONTOOLKIT_HPP
#include "IFunctionToolkit.hpp"

namespace INSTINCT_AGENT_NS {
    class BaseFunctionToolkit: public virtual IFunctionToolKit, public BaseStepFunction {
    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            const auto invocation = input->RequireMessage<FunctionToolInvocation>();
            const auto& fn_name = invocation.name();
            const auto fn_tool = this->LookupFunctionTool({.by_name = fn_name});
            if (!fn_tool) {
                FunctionToolResult failed_result;
                failed_result.set_exception("Failed to lookup function tool in this toolkit. Target function name: " + fn_name);
                failed_result.set_invocation_id(invocation.id());
                input->ProduceMessage(failed_result);
            }
            return fn_tool->Invoke(input);
        }
    };

    using FunctionToolkitPtr = std::shared_ptr<BaseFunctionToolkit>;
}





#endif //BASEFUNCTIONTOOLKIT_HPP
