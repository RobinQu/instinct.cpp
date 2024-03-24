//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_PROTOBUFUTILS_HPP
#define INSTINCT_PROTOBUFUTILS_HPP

#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    class ProtobufUtils final {

        template<class T>
        requires is_pb_message<T>
        static T ConvertJSONObjectToMessage(const nlohmann::json& json_object) {
            // TODO write this
            T result;
            return result;
        }


    };
}

#endif //INSTINCT_PROTOBUFUTILS_HPP
