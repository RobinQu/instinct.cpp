//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_JSONCONTEXTPOLICY_HPP
#define INSTINCT_JSONCONTEXTPOLICY_HPP

#include "CoreGlobals.hpp"
#include "IContext.hpp"

namespace INSTINCT_CORE_NS {
    class JSONContextPolicy {
    public:
        using PayloadType = nlohmann::json;
        class Manager {
        public:
            template<typename T>
            static T RequirePrimitive(const PayloadType& payload, const std::string& name) {
                return payload.at(name).get<T>();
            }

            template<typename T>
            static T RequirePrimitiveAt(const PayloadType& payload, const std::string& data_path) {
                return payload.at(nlohmann::json::json_pointer {data_path}).get<T>();
            }

            template<typename T>
            static void PutPrimitiveAt(PayloadType& payload, const std::string& data_path, T&& value) {
                payload.at(nlohmann::json::json_pointer {data_path}) = value;
            }

            template<typename T>
            static void PutPrimitive(PayloadType& payload, const std::string& name, T&& value) {
                payload.at(name) = value;
            }

            static void PutObject(PayloadType& payload, const std::string& name, const PayloadType& child_object) {
                payload[name] = child_object;
            }

            static void MergeContext(PayloadType& payload, const PayloadType& child_context) {
                payload.merge_patch(child_context);
            }

            static bool Contains(const PayloadType& payload, const std::string& name) {
                return payload.contains(name);
            }

        };
        using ManagerType = Manager;
    };

    using JSONContextPtr = ContextPtr<JSONContextPolicy>;

    static JSONContextPtr CreateJSONContext(const std::string& json_string = "") {
        nlohmann::json json_obj = json_string.empty() ?
                nlohmann::json {} :
                nlohmann::json::parse(json_string);
        return std::make_shared<IContext<JSONContextPolicy>>(json_obj);
    }
}


#endif //INSTINCT_JSONCONTEXTPOLICY_HPP
