//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/dynamic_message.h>

namespace INSTINCT_CORE_NS {
    using namespace google::protobuf;


    class ProtobufUtils final {
    public:

        template<class T>
        requires IsProtobufMessage<T>
        static void ConvertJSONObjectToMessage(const nlohmann::json& json_object, T& message) {
            auto *descriptor = message.GetDescriptor();
            auto *reflection = message.GetReflection();
            for (const auto& [name,v]: json_object.items()) {
                auto *field_descriptor = descriptor->FindFieldByName(name);
                assert_true(field_descriptor, "should have found field_ descriptor by column name: " + name);
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_STRING:
                        reflection->SetString(&message, field_descriptor, v.get<std::string>());
                        break;
                    case FieldDescriptor::CPPTYPE_INT32:
                        reflection->SetInt32(&message, field_descriptor, v.get<int>());
                        break;
                    case FieldDescriptor::CPPTYPE_INT64:
                        reflection->SetInt64(&message, field_descriptor, v.get<long>());
                        break;
                    case FieldDescriptor::CPPTYPE_UINT32:
                        reflection->SetUInt32(&message, field_descriptor, v.get<u_int32_t>());
                        break;
                    case FieldDescriptor::CPPTYPE_UINT64:
                        reflection->SetUInt64(&message, field_descriptor, v.get<u_int64_t>());
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                        reflection->SetDouble(&message, field_descriptor, v.get<double>());
                        break;
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        reflection->SetFloat(&message, field_descriptor, v.get<float>());
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        reflection->SetFloat(&message, field_descriptor, v.get<bool>());
                        break;
                    case FieldDescriptor::CPPTYPE_ENUM:
                        const auto enum_name = v.get<std::string>();
                        auto enum_value_descriptor = field_descriptor->enum_type()->FindValueByName();
                        assert_true(enum_value_descriptor, "should have found enum value by name " + enum_name);
                        reflection->SetEnum(&message, field_descriptor, enum_value_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_MESSAGE:
                        google::protobuf::DynamicMessageFactory dmf;
                        google::protobuf::Message* actual_msg = dmf.GetPrototype(field_descriptor)->New();
                        ConvertJSONObjectToMessage(v, *actual_msg);
                        reflection->SetAllocatedMessage(&message, actual_msg, field_descriptor);
                        break;
                    default:
                        throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
                                                            field_descriptor->name(),
                                                            field_descriptor->cpp_type_name()));
                }
            }
        }

        template<class T>
        requires IsProtobufMessage<T>
        static void ConvertMessageTOJsonObject(const T& message, nlohmann::json& json_object) {
            auto *descriptor = message.GetDescriptor();
            auto *reflection = message.GetReflection();
            for(int i=0;i<descriptor->field_count();++i) {
                auto* field_descriptor = descriptor->field(i);
                const auto field_name = descriptor->name();
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_STRING:
                        json_object[field_name] = reflection->GetString(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_INT32:
                        json_object[field_name] = reflection->GetInt32(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_INT64:
                        json_object[field_name] = reflection->GetInt64(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_UINT32:
                        json_object[field_name] = reflection->GetUInt32(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_UINT64:
                        json_object[field_name] = reflection->GetInt64(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_DOUBLE:
                        json_object[field_name] = reflection->GetDouble(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_FLOAT:
                        json_object[field_name] = reflection->GetFloat(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_BOOL:
                        json_object[field_name] = reflection->GetBool(message, field_descriptor);
                        break;
                    case FieldDescriptor::CPPTYPE_ENUM:
                        auto enum_value_descriptor = reflection->GetEnum(message, field_descriptor);
                        auto enum_name = enum_value_descriptor->name();
                        json_object[field_name] = enum_name;
                        break;
                    case FieldDescriptor::CPPTYPE_MESSAGE:
                        auto& sub_message = reflection->GetMessage(message, field_descriptor);
                        ConvertMessageTOJsonObject(sub_message, json_object[field_name]);
                        break;
                    default:
                        throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
                                                            field_descriptor->name(),
                                                            field_descriptor->cpp_type_name()));
                }
            }
        }

        template<typename T>
        requires IsProtobufMessage<T>
        static T Deserialize(const std::string& buf) {
            T result;
            util::JsonParseOptions options;
            options.ignore_unknown_fields = true;
            options.case_insensitive_enum_parsing = true;
            auto status = util::JsonStringToMessage(buf, &result, options);
            if (!status.ok()) {
                LOG_DEBUG("Deserialize failed. reason: {}, orginal string: {}", status.message().as_string(), buf);
            }
            assert_true(status.ok(), "failed to parse protobuf message from response body");
            return result;
        }

        template<typename T>
        requires IsProtobufMessage<T>
        static std::string Serialize(const T& obj) {
            std::string param_string;
            util::JsonPrintOptions json_print_options;
            json_print_options.preserve_proto_field_names = true;
            auto status = util::MessageToJsonString(obj, &param_string, json_print_options);
            if (!status.ok()) {
                LOG_DEBUG("Serialize failed message obj. reason: {}, original string: {}", status.message().as_string(), obj.DebugString());
            }
            assert_true(status.ok(), "failed to dump parameters from protobuf message");
            return param_string;
        }


    };
}

#endif //INSTINCT_PROTOBUFUTILS_HPP
