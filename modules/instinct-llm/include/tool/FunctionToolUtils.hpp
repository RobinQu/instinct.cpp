//
// Created by RobinQu on 2024/5/3.
//

#ifndef FUNCTIONTOOLUTILS_HPP
#define FUNCTIONTOOLUTILS_HPP

#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    class FunctionToolUtils final {
    public:
        // static void Convert(const std::vector<FunctionToolSchema> &function_tool_schema, std::vector<OpenAIChatCompletionRequest_ChatCompletionTool>& function_tools) {
        //     for (const auto& tool_schema: function_tool_schema) {
        //         OpenAIChatCompletionRequest_ChatCompletionTool tool;
        //         tool.set_type("function");
        //         FunctionTool* function_tool = tool.mutable_function();
        //         function_tool->set_description(tool_schema.description());
        //         function_tool->set_name(tool_schema.name());
        //         for(const auto& arg: tool_schema.arguments()) {
        //             const auto param = function_tool->mutable_parameters();
        //             param->set_type("object");
        //             FunctionTool_FunctionParametersSchema_FunctionParameterSchema parameter_schema;
        //             parameter_schema.set_type(arg.type());
        //             parameter_schema.set_description(arg.description());
        //             param->mutable_properties()->emplace(arg.name(), parameter_schema);
        //             if (arg.required()) {
        //                 *function_tool->mutable_required()->Add() = arg.name();
        //             }
        //         }
        //         function_tools.push_back(tool);
        //     }
        // }
        //
        // template<typename R>
        // requires RangeOf<R, FunctionTool>
        // static void Convert(R&& function_tools, std::vector<FunctionToolSchema> &function_tool_schema) {
        //
        // }


    };
}


#endif //FUNCTIONTOOLUTILS_HPP
