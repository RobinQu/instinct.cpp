//
// Created by RobinQu on 2024/4/9.
//

#ifndef PROTOMESSAGEFUNCTIONTOOL_HPP
#define PROTOMESSAGEFUNCTIONTOOL_HPP


#include "AgentGlobals.hpp"

namespace INSTINCT_AGENT_NS {
    /**
     * Tool class that support protobuf message as input and output.
     * Output should always be `std::string` and formated for easy understanding by LLM.
     * @tparam Input
     */
    template<typename Input>
    class ProtoMessageFunctionTool: public BaseFunctionTool {
        FunctionToolSchema schema_;
    public:
        explicit ProtoMessageFunctionTool(const FunctionToolOptions &options = {})
            : BaseFunctionTool(options),
              schema_() {
            // TODO compute schema
        }

        [[nodiscard]] const FunctionToolSchema & GetSchema() const override {
            return schema_;
        }

        std::string Execute(const std::string &action_input) override {
            auto input = ProtobufUtils::Deserialize<Input>(action_input);
            return DoExecute(input);
        }

        virtual std::string DoExecute(const Input& input) = 0;
    };
}

#endif //PROTOMESSAGEFUNCTIONTOOL_HPP
