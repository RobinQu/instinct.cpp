//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"
#include "tools/Assertions.hpp"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/message.h>


namespace INSTINCT_CORE_NS {
    using namespace google::protobuf;

    namespace details {
        static bool is_protobuf_struct_type(const FieldDescriptor* field_descriptor);
        static void convert_protobuf_struct_to_json_object(const google::protobuf::Message& message, nlohmann::ordered_json& output_json);

    }

    struct ToJsonObjectOptions {
        /**
         * Omit fields with default values to json object if this is set to `false`. Similar to:
         * google::protobuf::util::JsonPrintOptions::always_print_primitive_fields
         */
        bool keep_default_values = false;
    };

    static void assert_status_ok(const util::Status& status, const std::string& msg = "") {
        assert_true(status.ok(), msg.empty() ? "Protobuf function returned with error status: " + status.message().as_string() : msg);
    }

    class ProtobufUtils final {
    public:
        static const FieldDescriptor* WhichOneof(const Message& message,
                                          const std::string& oneof_name) {
            const Descriptor* descriptor = message.GetDescriptor();
            if (descriptor == nullptr) return nullptr;  // Internal error.

            const OneofDescriptor* oneof_descriptor =
                descriptor->FindOneofByName(oneof_name);
            if (oneof_descriptor == nullptr) return nullptr;  // No found with oneof_name.

            const Reflection* reflection = message.GetReflection();
            if (reflection == nullptr) return nullptr;  // Internal error.

            // Return nullptr if no oneof set.
            return reflection->GetOneofFieldDescriptor(message, oneof_descriptor);
        }

        template<class T>
        requires IsProtobufMessage<T>
        static void ConvertJSONObjectToMessage(const nlohmann::json& json_object, T* message) {
            // TODO use reflection instread of JsonStringToMessage
            util::JsonParseOptions options;
            options.ignore_unknown_fields = true;
            options.case_insensitive_enum_parsing = true;
            auto status = util::JsonStringToMessage(json_object.dump(), message, options);
            assert_status_ok(status);
        }


        //
        // template<class T>
        // requires IsProtobufMessage<T>
        // static void ConvertJSONObjectToMessage(const nlohmann::json& json_object, T& message) {
        //     auto *descriptor = message.GetDescriptor();
        //     auto *reflection = message.GetReflection();
        //     for (const auto& [name,v]: json_object.items()) {
        //         LOG_DEBUG("k={}, v.type={}", name, v.type_name());
        //         auto *field_descriptor = descriptor->FindFieldByName(name);
        //         assert_true(field_descriptor, "should have found field_ descriptor by column name: " + name);
        //         switch (field_descriptor->cpp_type()) {
        //             case FieldDescriptor::CPPTYPE_STRING: {
        //                 reflection->SetString(&message, field_descriptor, v.get<std::string>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_INT32: {
        //                 reflection->SetInt32(&message, field_descriptor, v.get<int>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_INT64: {
        //                 reflection->SetInt64(&message, field_descriptor, v.get<long>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_UINT32: {
        //                 reflection->SetUInt32(&message, field_descriptor, v.get<u_int32_t>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_UINT64: {
        //                 reflection->SetUInt64(&message, field_descriptor, v.get<u_int64_t>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_DOUBLE: {
        //                 reflection->SetDouble(&message, field_descriptor, v.get<double>());
        //                     break;
        //             }
        //             case FieldDescriptor::CPPTYPE_FLOAT: {
        //                 reflection->SetFloat(&message, field_descriptor, v.get<float>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_BOOL: {
        //                 reflection->SetBool(&message, field_descriptor, v.get<bool>());
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_ENUM: {
        //                 const auto enum_name = v.get<std::string>();
        //                 auto enum_value_descriptor = field_descriptor->enum_type()->FindValueByName();
        //                 assert_true(enum_value_descriptor, "should have found enum value by name " + enum_name);
        //                 reflection->SetEnum(&message, field_descriptor, enum_value_descriptor);
        //                 break;
        //             }
        //             case FieldDescriptor::CPPTYPE_MESSAGE: {
        //                 if (field_descriptor->is_map()) {
        //                     LOG_WARN("map found, but not supported yet");
        //                 } else {
        //
        //                     DynamicMessageFactory dmf;
        //                     Message* actual_msg = dmf.GetPrototype(field_descriptor)->New();
        //                     ConvertJSONObjectToMessage(v, *actual_msg);
        //                     reflection->SetAllocatedMessage(&message, actual_msg, field_descriptor);
        //                     break;
        //                 }
        //             }
        //             default: {
        //                 throw InstinctException(fmt::format("Unknown cpp_type for this field. name={}, cpp_type={}",
        //                                                     field_descriptor->name(),
        //                                                     field_descriptor->cpp_type_name()));
        //             }
        //         }
        //     }
        // }


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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || StringUtils::IsNotBlankString(v)) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v!=0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v!=0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v!= 0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v != 0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v!=0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v != 0) {
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
                            if (options.keep_default_values || reflection->HasField(message, field_descriptor) || v == true) {
                                json_object[field_name] = v;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_ENUM: {
                        if (field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                auto enum_value_descriptor = reflection->GetRepeatedEnum(message, field_descriptor, j);
                                auto enum_name = enum_value_descriptor->name();
                                json_object[field_name].push_back(enum_name);
                            }
                        } else {
                            if (auto enum_value_descriptor = reflection->GetEnum(message, field_descriptor); options.keep_default_values || reflection->HasField(message, field_descriptor) || enum_value_descriptor->number() != 0) {
                                auto enum_name = enum_value_descriptor->name();
                                json_object[field_name] = enum_name;
                            }
                        }
                        break;
                    }

                    case FieldDescriptor::CPPTYPE_MESSAGE: {
                        if(field_descriptor->is_repeated()) {
                            for(int j=0;j<repeated_field_size;++j) {
                                auto& message_item = reflection->GetRepeatedMessage(message, field_descriptor, j);
                                if (field_descriptor->is_map()) {
                                    auto* entry_descriptor = message_item.GetDescriptor();
                                    auto* entry_reflection = message_item.GetReflection();
                                    auto key_name = entry_reflection->GetString(message_item, entry_descriptor->map_key());

                                    if (entry_descriptor->map_value()->type() == FieldDescriptor::TYPE_MESSAGE) {
                                        auto& value_message = entry_reflection->GetMessage(message_item, entry_descriptor->map_value());
                                        nlohmann::ordered_json sub_obj;
                                        ConvertMessageToJsonObject(value_message, sub_obj, options);
                                        json_object[field_name][key_name] = sub_obj;
                                    } else {
                                        LOG_ERROR("Type of entry value should be message. Other type is not supported.");
                                    }
                                } else {
                                    nlohmann::ordered_json sub_obj;
                                    ConvertMessageToJsonObject(message_item, sub_obj, options);
                                    json_object[field_name].push_back(sub_obj);
                                }
                            }
                        } else {
                            if (reflection->HasField(message, field_descriptor)) {
                                // TODO support other well-known types
                                nlohmann::ordered_json sub_obj;
                                auto& sub_message = reflection->GetMessage(message, field_descriptor);
                                if (details::is_protobuf_struct_type(field_descriptor)) {
                                    details::convert_protobuf_struct_to_json_object(sub_message, sub_obj);
                                } else {
                                    ConvertMessageToJsonObject(sub_message, sub_obj, options);
                                }
                                json_object[field_name] = sub_obj;
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

        static void Deserialize(const std::string& buf, Message& result) {
            util::JsonParseOptions options;
            options.ignore_unknown_fields = true;
            options.case_insensitive_enum_parsing = true;
            const auto status = util::JsonStringToMessage(buf, &result, options);
            if (!status.ok()) {
                LOG_DEBUG("Deserialize failed. reason: {}, orginal string: {}", status.message().as_string(), buf);
            }
            assert_true(status.ok(), "failed to parse protobuf message from response body");
        }

        static void Serialize(const Message& obj, std::string& param_string) {
            util::JsonPrintOptions json_print_options;
            json_print_options.preserve_proto_field_names = true;
            // json_print_options.always_print_primitive_fields = true;
            const auto status = util::MessageToJsonString(obj, &param_string, json_print_options);
            if (!status.ok()) {
                LOG_DEBUG("Serialize failed message obj. reason: {}, original string: {}", status.message().as_string(), obj.DebugString());
            }
            assert_true(status.ok(), "failed to dump parameters from protobuf message");
        }

        static std::string Serialize(const Message& obj) {
            std::string param_string;
            util::JsonPrintOptions json_print_options;
            json_print_options.preserve_proto_field_names = true;
            // json_print_options.always_print_primitive_fields = true;
            const auto status = util::MessageToJsonString(obj, &param_string, json_print_options);
            if (!status.ok()) {
                LOG_DEBUG("Serialize failed message obj. reason: {}, original string: {}", status.message().as_string(), obj.DebugString());
            }
            assert_true(status.ok(), "failed to dump parameters from protobuf message");
            return param_string;
        }


    };


    namespace details {
        static bool is_protobuf_struct_type(const FieldDescriptor* field_descriptor) {
            return field_descriptor->message_type()->full_name() == "google.protobuf.Struct";
        }

        static bool convert_protobuf_struct_value_to_json_object(const google::protobuf::Message& value_message, nlohmann::ordered_json& value_json) { // NOLINT(*-no-recursion)
            auto* value_message_reflection = value_message.GetReflection();
            auto* value_mesage_actual_oneof_field_descriptor = ProtobufUtils::WhichOneof(value_message, "kind");
            if (!value_mesage_actual_oneof_field_descriptor) { // it's possible oneof is not set, then do nothing
                return false;
            }
            auto& value_actual_oneof_name = value_mesage_actual_oneof_field_descriptor->name();
            if (value_actual_oneof_name == "null_value") {
                value_json = nullptr;
            }
            if (value_actual_oneof_name == "number_value") {
                // double v = value_message_reflection->GetDouble(value_message, value_mesage_actual_oneof_field_descriptor);;
                // auto int_v = static_cast<int32_t>(v);
                //
                // const nlohmann::json number_obj = int_v;
                // std::cout << (int_v == v) << " == " << number_obj.dump() << std::endl;
                // value_json = number_obj;
                value_json = value_message_reflection->GetDouble(value_message, value_mesage_actual_oneof_field_descriptor);
            }
            if (value_actual_oneof_name == "string_value") {
                value_json = value_message_reflection->GetString(value_message, value_mesage_actual_oneof_field_descriptor);
            }
            if (value_actual_oneof_name == "bool_value") {
                value_json = value_message_reflection->GetBool(value_message, value_mesage_actual_oneof_field_descriptor);
            }
            if (value_actual_oneof_name == "struct_value") {
                nlohmann::ordered_json sub_obj;
                convert_protobuf_struct_to_json_object(value_message_reflection->GetMessage(value_message, value_mesage_actual_oneof_field_descriptor), sub_obj);
                value_json =  sub_obj;
            }
            if (value_actual_oneof_name == "list_value") {
                auto& list_value_message = value_message_reflection->GetMessage(value_message, value_mesage_actual_oneof_field_descriptor);
                auto* list_value_message_descriptor = list_value_message.GetDescriptor();
                auto* list_value_message_reflection = list_value_message.GetReflection();

                auto* values_in_list_value_field_descriptor = list_value_message_descriptor->FindFieldByName("values");
                auto values_size = list_value_message_reflection->FieldSize(list_value_message, values_in_list_value_field_descriptor);
                for(int k=0;k<values_size;++k) {
                    auto& inner_value_message = list_value_message_reflection->GetRepeatedMessage(value_message, values_in_list_value_field_descriptor, k);
                    nlohmann::ordered_json sub_obj;
                    bool ok = convert_protobuf_struct_value_to_json_object(inner_value_message, sub_obj);
                    if (ok) {
                        value_json.push_back(sub_obj);
                    }
                }
            }
            return true;
        }

        static void convert_protobuf_struct_to_json_object(const google::protobuf::Message &message, // NOLINT(*-no-recursion)
                                                           nlohmann::ordered_json &output_json) {
            auto map_field_descriptor = message.GetDescriptor()->FindFieldByName("fields");
            auto map_field_reflcetion = message.GetReflection();
            auto map_entry_size = map_field_reflcetion->FieldSize(message, map_field_descriptor);
            for(int i=0;i<map_entry_size;++i) {
                auto& entry_message = map_field_reflcetion->GetRepeatedMessage(message, map_field_descriptor, i);
                auto* entry_descriptor = entry_message.GetDescriptor();
                auto* entry_reflection = entry_message.GetReflection();
                // map key should always be string for Struct
                auto key_name = entry_reflection->GetString(entry_message, entry_descriptor->map_key());
                auto& value_message = entry_reflection->GetMessage(entry_message, entry_descriptor->map_value());
                nlohmann::ordered_json value_json;
                bool ok = convert_protobuf_struct_value_to_json_object(value_message, value_json);
                if (ok) {
                    output_json[key_name] = value_json;
                }

            }
        }
    }
}

#endif //INSTINCT_PROTOBUFUTILS_HPP
