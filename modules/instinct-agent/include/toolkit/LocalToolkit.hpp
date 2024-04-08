//
// Created by RobinQu on 2024/4/8.
//

#ifndef LOCALTOOLKIT_HPP
#define LOCALTOOLKIT_HPP

#include "BaseFunctionToolkit.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * A function toolkit that manage local function tools
     */
    class LocalFunctionToolkit final: public BaseFunctionToolkit  {
        std::unordered_map<std::string, FunctionToolPtr> functions_map_;

    public:
        bool RegisterFunctionTool(const FunctionToolPtr &function_tool) override {
            const auto& fn_name = function_tool->GetSchema().name();
            if (functions_map_.contains(fn_name)) {
                return false;
            }
            functions_map_[fn_name] = function_tool;
            return true;
        }

        bool UnregisterFuncionTool(const std::string &name) override {
            if (!functions_map_.contains(name)) {
                return false;
            }
            return functions_map_.erase(name);
        }

        std::unordered_set<std::string> GetFunctionToolNames() override {
            auto names_view = std::ranges::keys_view(functions_map_);
            return {names_view.begin(), names_view.end()};
        }
    };
}

#endif //LOCALTOOLKIT_HPP
