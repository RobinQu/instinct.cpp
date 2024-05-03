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
        FunctionTool schema_;
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

        [[nodiscard]] const FunctionTool & GetSchema() const override {
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
            const auto* descriptor = Input::GetDescriptor();
            auto* properties = schema_.mutable_parameters()->mutable_properties();

            for(int i=0;i<descriptor->field_count();++i) {
                const auto& field_descriptor = descriptor->field(i);
                if (field_descriptor->has_optional_keyword() && !GetOptions().with_optional_arguments) {
                    // only output required fields
                    continue;
                }
                FunctionTool_FunctionParametersSchema_FunctionParameterSchema parameters_schema;
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_INT32:
                    case FieldDescriptor::CPPTYPE_INT64:
                    case FieldDescriptor::CPPTYPE_UINT32:
                    case FieldDescriptor::CPPTYPE_UINT64:
                        parameters_schema.set_type("integer");
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        parameters_schema.set_type("number");
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        parameters_schema.set_type("boolean");
                        break;
                    case FieldDescriptor::CPPTYPE_STRING:
                        parameters_schema.set_type("string");
                        break;
                    default:
                        // don't setting arg_type
                        break;
                }
                properties->emplace(field_descriptor->name(), parameters_schema);
            }
        }
    };


}

#endif //PROTOMESSAGEFUNCTIONTOOL_HPP
