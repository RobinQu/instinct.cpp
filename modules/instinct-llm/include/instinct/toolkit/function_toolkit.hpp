//
// Created by RobinQu on 2024/4/8.
//

#ifndef BASEFUNCTIONTOOLKIT_HPP
#define BASEFUNCTIONTOOLKIT_HPP
#include <instinct/toolkit/function_tool.hpp>

namespace INSTINCT_LLM_NS {
    struct FunctionToolLookupOptions {
        std::string by_name;
    };

    class IFunctionToolKit {
    public:
        IFunctionToolKit()=default;
        virtual ~IFunctionToolKit()=default;
        IFunctionToolKit(IFunctionToolKit&&)=delete;
        IFunctionToolKit(const IFunctionToolKit&)=delete;

        virtual bool RegisterFunctionTool(const FunctionToolPtr& function_tool) = 0;
        virtual bool UnregisterFunctionTool(const std::string& name) = 0;
        virtual std::unordered_set<std::string> GetFunctionToolNames() = 0;
        virtual FunctionToolPtr LookupFunctionTool(const FunctionToolLookupOptions& options) = 0;
        [[nodiscard]] virtual const std::vector<FunctionTool>& GetAllFunctionToolSchema() const = 0;
    };


    class BaseFunctionToolkit: public virtual IFunctionToolKit, public BaseRunnable<ToolCallObject, FunctionToolResult> {
    public:
        FunctionToolResult Invoke(const ToolCallObject &invocation) override {
            const auto& fn_name = invocation.function().name();
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
