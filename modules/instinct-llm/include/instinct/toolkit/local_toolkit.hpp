//
// Created by RobinQu on 2024/4/8.
//

#ifndef LOCALTOOLKIT_HPP
#define LOCALTOOLKIT_HPP

#include <instinct/toolkit/function_toolkit.hpp>

namespace INSTINCT_LLM_NS {
    /**
     * A function toolkit that manage local function tools
     */
    class LocalFunctionToolkit final: public BaseFunctionToolkit  {
        std::unordered_map<std::string, FunctionToolPtr> functions_map_;
        std::vector<FunctionTool> function_tools_;

    public:
        [[nodiscard]] const std::vector<FunctionTool>& GetAllFunctionToolSchema() const override  {
            return function_tools_;
        }

        bool RegisterFunctionTool(const FunctionToolPtr &function_tool) override {
            const auto& fn_name = function_tool->GetSchema().name();
            if (functions_map_.contains(fn_name)) {
                return false;
            }
            functions_map_[fn_name] = function_tool;
            function_tools_.push_back(function_tool->GetSchema());
            return true;
        }

        bool UnregisterFunctionTool(const std::string &name) override {
            if (!functions_map_.contains(name)) {
                return false;
            }
            std::erase_if(function_tools_, [&](const FunctionTool& tool) {
                return tool.name() == name;
            });
            return functions_map_.erase(name);
        }

        std::unordered_set<std::string> GetFunctionToolNames() override {
            auto names_view = std::views::keys(functions_map_);
            return {names_view.begin(), names_view.end()};
        }

        FunctionToolPtr LookupFunctionTool(const FunctionToolLookupOptions &options) override {
            assert_true(StringUtils::IsNotBlankString(options.by_name), "Lookup by name is supported only.");
            if (functions_map_.contains(options.by_name)) {
                return functions_map_.at(options.by_name);
            }
            return nullptr;

        }
    };

    template<typename ...Args>
    static FunctionToolkitPtr CreateLocalToolkit(Args... args) {
        auto tk = std::make_shared<LocalFunctionToolkit>();
        (void)std::initializer_list<int> {(tk->RegisterFunctionTool(args), 0)...};
        return tk;
    }

    /**
     * Create toolkit with given tools
     * @param tools
     * @return
     */
    static FunctionToolkitPtr CreateLocalToolkit(const std::vector<FunctionToolPtr>& tools) {
        auto tk = std::make_shared<LocalFunctionToolkit>();
        for(const auto& tool: tools) {
            tk->RegisterFunctionTool(tool);
        }
        return tk;
    }
}

#endif //LOCALTOOLKIT_HPP
