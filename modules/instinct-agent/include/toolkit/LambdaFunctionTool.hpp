//
// Created by RobinQu on 2024/4/8.
//

#ifndef LOCALFUNCTIONTOOL_HPP
#define LOCALFUNCTIONTOOL_HPP

#include <utility>

#include "BaseFunctionTool.hpp"

namespace INSTINCT_AGENT_NS {
    using FunctionToolFn = std::function<std::string(const std::string&)>;

    class LambdaFunctionTool final: public BaseFunctionTool {
        FunctionToolFn fn_;

    public:
        LambdaFunctionTool(const FunctionToolSchema &schema, FunctionToolFn fn)
            : BaseFunctionTool(schema),
              fn_(std::move(fn)) {
        }

        std::string Execute(const std::string &action_input) override {
            return std::invoke(fn_, action_input);
        }
    };
}


#endif //LOCALFUNCTIONTOOL_HPP
