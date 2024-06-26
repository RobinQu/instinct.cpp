//
// Created by RobinQu on 2024/4/8.
//

#ifndef IFUNCTIONTOOLKIT_HPP
#define IFUNCTIONTOOLKIT_HPP


#include <instinct/LLMGlobals.hpp>
#include <instinct/toolkit/BaseFunctionTool.hpp>


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


}

#endif //IFUNCTIONTOOLKIT_HPP
