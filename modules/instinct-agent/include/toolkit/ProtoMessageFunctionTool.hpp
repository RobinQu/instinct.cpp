//
// Created by RobinQu on 2024/4/9.
//

#ifndef PROTOMESSAGEFUNCTIONTOOL_HPP
#define PROTOMESSAGEFUNCTIONTOOL_HPP


#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Tool class that support protobuf message as input and output
     * @tparam Input
     * @tparam Output
     */
    template<typename Input, typename Output>
    class ProtoMessageFunctionTool: public BaseFunctionTool {
        FunctionToolSchema schema_;
    public:
        ProtoMessageFunctionTool(): BaseFunctionTool() {
        }

        [[nodiscard]] const FunctionToolSchema & GetSchema() const override {
            return schema_;
        }

        std::string Execute(const std::string &action_input) override {
            auto input = ProtobufUtils::Deserialize<Output>(action_input);
            auto output = DoExecute(input);
            return ProtobufUtils::Serialize(output);
        }

        virtual Output DoExecute(const Input& input) = 0;
    };
}

#endif //PROTOMESSAGEFUNCTIONTOOL_HPP
