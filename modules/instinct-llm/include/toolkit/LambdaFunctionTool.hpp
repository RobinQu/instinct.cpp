//
// Created by RobinQu on 2024/4/8.
//

#ifndef LOCALFUNCTIONTOOL_HPP
#define LOCALFUNCTIONTOOL_HPP

#include <utility>

#include "BaseFunctionTool.hpp"

namespace INSTINCT_LLM_NS {
    using FunctionToolFn = std::function<std::string(const std::string&)>;

    class LambdaFunctionTool final: public BaseFunctionTool {
        FunctionToolFn fn_;
        FunctionToolSchema schema_;

    public:
        LambdaFunctionTool(const FunctionToolSchema &schema, FunctionToolFn fn)
            : BaseFunctionTool(), fn_(std::move(fn)), schema_(schema) {
        }

        [[nodiscard]] const FunctionToolSchema & GetSchema() const override {
            return schema_;
        }

        std::string Execute(const std::string &action_input) override {
            return std::invoke(fn_, action_input);
        }
    };

    static FunctionToolPtr CreateFunctionTool(const FunctionToolSchema& schema, FunctionToolFn fn) {
        return std::make_shared<LambdaFunctionTool>(
            schema,
            std::move(fn)
        );
    }
}


#endif //LOCALFUNCTIONTOOL_HPP
