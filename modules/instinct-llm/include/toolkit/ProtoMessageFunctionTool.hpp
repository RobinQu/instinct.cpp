//
// Created by RobinQu on 2024/4/9.
//

#ifndef PROTOMESSAGEFUNCTIONTOOL_HPP
#define PROTOMESSAGEFUNCTIONTOOL_HPP


#include "LLMGlobals.hpp"
#include "toolkit/BaseFunctionTool.hpp"

namespace INSTINCT_LLM_NS {
    /**
     * Tool class that support protobuf message as input and output.
     * Output should always be `std::string` and formated for easy understanding by LLM.
     * @tparam Input
     */
    template<typename Input, typename Output>
    requires IsProtobufMessage<Input> && IsProtobufMessage<Output>
    class ProtoMessageFunctionTool: public BaseFunctionTool {
        FunctionToolSchema schema_;
    public:
        explicit ProtoMessageFunctionTool(
            const std::string& name,
            const std::string& description,
            const FunctionToolOptions &options)
            : BaseFunctionTool(options),
              schema_() {
            schema_.set_name(name);
            schema_.set_description(description);
            GenerateFunctionSchema_();
        }

        [[nodiscard]] const FunctionToolSchema & GetSchema() const override {
            return schema_;
        }

        /**
         * Read input as JSON string and write output as JSON string
         * @param action_input
         * @return
         */
        std::string Execute(const std::string &action_input) override {
            auto input = ProtobufUtils::Deserialize<Input>(action_input);
            auto result = DoExecute(input);
            return ProtobufUtils::Serialize(result);
        }

    protected:
        virtual Output DoExecute(const Input& input) = 0;

    private:
        void GenerateFunctionSchema_() {
            Input target;
            const auto* descriptor = target.GetDescriptor();
            for(int i=0;i<descriptor->field_count();++i) {
                const auto& field_descriptor = descriptor->field(i);
                if (field_descriptor->has_optional_keyword() && !GetOptions().with_optional_arguments) {
                    // only output required fields
                    continue;
                }
                auto* arg = schema_.add_arguments();
                arg->set_name(field_descriptor->name());
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_INT32:
                    case FieldDescriptor::CPPTYPE_INT64:
                    case FieldDescriptor::CPPTYPE_UINT32:
                    case FieldDescriptor::CPPTYPE_UINT64:
                        arg->set_type("integer");
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        arg->set_type("number");
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        arg->set_type("boolean");
                        break;
                    case FieldDescriptor::CPPTYPE_STRING:
                        arg->set_type("string");
                        break;
                    default:
                        // don't setting arg_type
                        break;
                }
            }
        }
    };


}

#endif //PROTOMESSAGEFUNCTIONTOOL_HPP
