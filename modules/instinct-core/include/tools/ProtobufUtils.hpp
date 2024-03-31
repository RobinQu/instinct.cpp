//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"
#include <google/protobuf/util/json_util.h>

namespace INSTINCT_CORE_NS {
    using namespace google::protobuf;


    class ProtobufUtils final {
    public:

        template<class T>
        requires is_pb_message<T>
        static T ConvertJSONObjectToMessage(const nlohmann::json& json_object) {
            // TODO write this
            T result;
            return result;
        }

        template<typename T>
        requires is_pb_message<T>
        static T Deserialize(const std::string& buf) {
            T result;
            auto status = util::JsonStringToMessage(buf, &result);
            if (!status.ok()) {
                LOG_DEBUG("Deserialize failed string: {}", buf);
            }
            assert_true(status.ok(), "failed to parse protobuf message from response body");
            return result;
        }

        template<typename T>
        requires is_pb_message<T>
        static std::string Serialize(const T& obj) {
            std::string param_string;
            util::JsonPrintOptions json_print_options;
            json_print_options.preserve_proto_field_names = true;
            auto status = util::MessageToJsonString(obj, &param_string, json_print_options);
            if (!status.ok()) {
                LOG_DEBUG("Serialize failed message obj: {}", obj.DebugString());
            }
            assert_true(status.ok(), "failed to dump parameters from protobuf message");
            return param_string;
        }


    };
}

#endif //INSTINCT_PROTOBUFUTILS_HPP
