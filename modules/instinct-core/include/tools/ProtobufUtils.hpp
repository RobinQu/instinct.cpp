//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"
#include <google/protobuf/util/json_util.h>

namespace INSTINCT_CORE_NS {
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
            auto status = google::protobuf::util::JsonStringToMessage(buf, &result);
            assert_true(status.ok(), "failed to parse protobuf message from response body");
            return result;
        }

        template<typename T>
        requires is_pb_message<T>
        static std::string Serialize(const T& obj) {
            std::string param_string;
            auto status = google::protobuf::util::MessageToJsonString(obj, &param_string);
            assert_true(status.ok(), "failed to dump parameters from protobuf message");
            return param_string;
        }


    };
}

#endif //INSTINCT_PROTOBUFUTILS_HPP
