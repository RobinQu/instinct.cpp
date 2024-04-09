//
// Created by RobinQu on 2024/4/9.
//

#ifndef AGENTTESTGLOBALS_HPP
#define AGENTTESTGLOBALS_HPP


#include "AgentGlobals.hpp"
#include "toolkit/LambdaFunctionTool.hpp"

namespace INSTINCT_AGENT_NS::test {

    class SearchTool final: public BaseFunctionTool {
        FunctionToolSchema& schema_;

    public:
        SearchTool(): BaseFunctionTool(), schema_() {
            auto *arg = schema_.add_arguments();
            arg->set_name("query");
            arg->set_type(VARCHAR);
        }

        [[nodiscard]] const FunctionToolSchema & GetSchema() const override {
            return schema_;
        }

        std::string Execute(const std::string &action_input) override {

        }
    };

}


#endif //AGENTTESTGLOBALS_HPP
