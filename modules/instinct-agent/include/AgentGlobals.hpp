//
// Created by RobinQu on 2024/4/6.
//

#ifndef AGENTGLOBALS_HPP
#define AGENTGLOBALS_HPP


#define INSTINCT_AGENT_NS instinct::agent
#include <string>

#include "CoreGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "toolkit/BaseFunctionTool.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    using Planer = MessageChain<AgentState, AgentThoughtMessage>;
    using PlannerPtr = MessageChainPtr<AgentState, AgentThoughtMessage>;

    /**
     * Render arguments of function tools according to JSON Schema.
     * @tparam T range container
     * @param args range of `FunctionToolArgument`
     * @return
     */
    template<typename T>
    requires RangeOf<T, FunctionToolArgument>
    static std::string RenderFunctionToolArgument(T&& args) {
        auto args_view = args | std::views::transform([](const FunctionToolArgument& arg) {
            // translate to python type which most LLM is more familar with
            std::string arg_type_string;
            const auto& pt = arg.type();
            if (pt == INT32) arg_type_string = "int";
            if (pt == INT64) arg_type_string = "int";
            if (pt == FLOAT) arg_type_string = "float";
            if (pt == DOUBLE) arg_type_string = "float";
            if (pt == BOOL) arg_type_string = "bool";
            if (pt == VARCHAR) arg_type_string = "string";
            return "{\"" +  arg.name() + ":\"" + arg_type_string + "\"}";
        });
        return  "{" + StringUtils::JoinWith(args_view, ",") + "}";
    }

    /**
     * Render function tool descriptions with given schema.
     * @tparam T
     * @param tools
     * @param with_args
     * @return
     */
    template<typename T>
    requires RangeOf<T, FunctionToolSchema>
    static std::string RenderFunctionTools(T&& tools, const bool with_args = true) {
        auto fn_desc_view = tools | std::views::transform([&](const FunctionToolSchema& fn_schema) {
            if (with_args) {
                return fmt::format("{}: {}. args={}", fn_schema.name(), fn_schema.description(), RenderFunctionToolArgument(fn_schema.arguments()));
            }
            return fmt::format("{}: {}", fn_schema.name(), fn_schema.description());
        });
        return StringUtils::JoinWith(fn_desc_view, "\n");
    }




}



#endif //AGENTGLOBALS_HPP
