//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"
#include "tools/Assertions.hpp"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/dynamic_message.h>

namespace INSTINCT_CORE_NS {
    using namespace google::protobuf;


    struct ToJsonObjectOptions {
        // As enum is written as string, this controls it should be written in camel-case or snake-case.
        bool camel_case_enum = false;
        // Don't write field with default values to json object if this is set to `false`.
        bool keep_default_values = false;
    };

    class ProtobufUtils final {
    public:
        template<class T>
        requires IsProtobufMessage<T>
        static void ConvertJSONObjectToMessage(const nlohmann::json& json_object, T& message) {
            auto *descriptor = message.GetDescriptor();
            auto *reflection = message.GetReflection();
            for (const auto& [name,v]: json_object.items()) {
                LOG_DEBUG("k={}, v.type={}", name, v.type_name());
                auto *field_descriptor = descriptor->FindFieldByName(name);
                assert_true(field_descriptor, "should have found field_ descriptor by column name: " + name);
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_STRING: {
                        reflection->SetString(&message, field_descriptor, v.get<std::string>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_INT32: {
                        reflection->SetInt32(&message, field_descriptor, v.get<int>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_INT64: {
                        reflection->SetInt64(&message, field_descriptor, v.get<long>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_UINT32: {
                        reflection->SetUInt32(&message, field_descriptor, v.get<u_int32_t>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_UINT64: {
                        reflection->SetUInt64(&message, field_descriptor, v.get<u_int64_t>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_DOUBLE: {
                        reflection->SetDouble(&message, field_descriptor, v.get<double>());
                            break;
                    }
                    case FieldDescriptor::CPPTYPE_FLOAT: {
                        reflection->SetFloat(&message, field_descriptor, v.get<float>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_BOOL: {
                        reflection->SetBool(&message, field_descriptor, v.get<bool>());
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_ENUM: {
                        const auto enum_name = v.get<std::string>();
                        auto enum_value_descriptor = field_descriptor->enum_type()->FindValueByName();
                        assert_true(enum_value_descriptor, "should have found enum value by name " + enum_name);
                        reflection->SetEnum(&message, field_descriptor, enum_value_descriptor);
                        break;
                    }
                    case FieldDescriptor::CPPTYPE_MESSAGE: {
                        if (field_descriptor->is_map()) {
                            LOG_WARN("map found, but not supported yet");
                        } else {

                            DynamicMessageFactory dmf;
                            Message* actual_msg = dmf.GetPrototype(field_descriptor)->New();
                            ConvertJSONObjectToMessage(v, *actual_msg);
                            reflection->SetAllocatedMessage(&message, actual_msg, field_descriptor);
                            break;
                        }
                    }
                    default: {
                        throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
                                                            field_descriptor->name(),
                                                            field_descriptor->cpp_type_name()));
                    }
                }
            }
        }


        template<class T>
        requires IsProtobufMessage<T>
        static void ConvertMessageToJsonObject(const T& message, nlohmann::ordered_json& json_object, const ToJsonObjectOptions& options = {}) {
            auto *descriptor = message.GetDescriptor();
            auto *reflection = message.GetReflection();
            for(int i=0;i<descriptor->field_count();++i) {
                auto* field_descriptor = descriptor->field(i);
                // LOG_DEBUG("k={}, index={}, number={}", field_descriptor->name(), field_descriptor->index(), field_descriptor->number());
                const auto field_name = field_descriptor->name();
                const int repeated_field_size = field_descriptor->is_repeated() ? reflection->FieldSize(message, field_descriptor) : 0;
                switch (field_descriptor->cpp_type()) {
                    case FieldDescriptor::CPPTYPE_STRING: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(reflection->GetRepeatedString(message, field_descriptor, j));
                            }
                        } else {

                            auto v = reflection->GetString(message, field_descriptor);
                            if (options.keep_default_values || StringUtils::IsNotBlankString(v)) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT32: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(reflection->GetRepeatedInt32(message, field_descriptor, j));
                            }
                        } else {
                            auto v = reflection->GetInt32(message, field_descriptor);;
                            if (v) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_INT64: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(reflection->GetRepeatedInt64(message, field_descriptor, j));
                            }
                        } else {
                            auto v = reflection->GetInt64(message, field_descriptor);;
                            if (options.keep_default_values || v!=0) {
                                json_object[field_name] = v;
                            }
                        }

                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT32: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(reflection->GetRepeatedUInt32(message, field_descriptor, j));
                            }
                        } else {
                            auto v = reflection->GetUInt32(message, field_descriptor);
                            if (options.keep_default_values || v!= 0) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_UINT64: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(
                                    reflection->GetRepeatedUInt64(message, field_descriptor, j)
                                );
                            }
                        } else {
                            auto v = reflection->GetUInt64(message, field_descriptor);;
                            if (options.keep_default_values || v != 0) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_DOUBLE: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(
                                    reflection->GetRepeatedDouble(message, field_descriptor, j)
                                );
                            }
                        } else {
                            auto v = reflection->GetDouble(message, field_descriptor);;
                            if (options.keep_default_values || v!=0) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_FLOAT: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(
                                    reflection->GetRepeatedFloat(message, field_descriptor, j)
                                );
                            }
                        } else {
                            auto v = reflection->GetFloat(message, field_descriptor);
                            if (options.keep_default_values || v != 0) {
                                json_object[field_name] = v;
                            }
                        }

                        break;
                    }

                    case FieldDescriptor::CPPTYPE_BOOL: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                json_object[field_name].push_back(
                                reflection->GetRepeatedBool(message, field_descriptor, j)
                                );
                            }
                        } else {
                            auto v = reflection->GetBool(message, field_descriptor);
                            if (options.keep_default_values || v == true) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                auto enum_value_descriptor = reflection->GetRepeatedEnum(message, field_descriptor, j);
                                // TODO add options for snake_case or CamelCase
                                auto enum_name = options.camel_case_enum ? enum_value_descriptor->name():  StringUtils::CamelToSnake(enum_value_descriptor->name());
                                json_object[field_name].push_back(enum_name);
                            }
                        } else {
                            if (auto enum_value_descriptor = reflection->GetEnum(message, field_descriptor); enum_value_descriptor->number() != 0) {
                                auto enum_name = options.camel_case_enum ? enum_value_descriptor->name() : StringUtils::CamelToSnake(enum_value_descriptor->name());
                                json_object[field_name] = enum_name;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: {
                        if(field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                auto& message_item = reflection->GetRepeatedMessage(message, field_descriptor, j);
                                nlohmann::ordered_json sub_obj;
                                ConvertMessageToJsonObject(message_item, sub_obj);
                                json_object[field_name].push_back(sub_obj);
                            }
                        } else {
                            if (field_descriptor->is_map()) {
                                LOG_DEBUG("map detected, but not supported");
                            } else {
                                if (reflection->HasField(message, field_descriptor)) {
                                    auto& sub_message = reflection->GetMessage(message, field_descriptor);
                                    nlohmann::ordered_json sub_obj;
                                    ConvertMessageToJsonObject(sub_message, sub_obj);
                                    json_object[field_name] = sub_obj;
                                }
                            }

                        }
                        break;
                    }

                    default: {
                        throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
                                                            field_descriptor->name(),
                                                            field_descriptor->cpp_type_name()));
                    }

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
