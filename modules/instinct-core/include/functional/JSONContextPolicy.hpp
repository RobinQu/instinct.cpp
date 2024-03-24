//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_JSONCONTEXTPOLICY_HPP
#define INSTINCT_JSONCONTEXTPOLICY_HPP

#include "CoreGlobals.hpp"
#include "IContext.hpp"
#include <google/protobuf/util/json_util.h>
#include "tools/Assertions.hpp"

namespace INSTINCT_CORE_NS {
    using namespace google::protobuf;
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
                payload[name] = value;
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

            template<typename T>
            static T RequireMessage(const PayloadType& payload, const std::string& name) {
                // TODO use reflection instead
                auto str = payload[name].dump();
                T result;
                auto status = util::JsonStringToMessage(str, &result);
                assert_true(status.ok());
                return result;
            }

            template<typename T>
            static void PutMessage(PayloadType& payload, const std::string& name, const T& message) {
                // TODO use reflection instead
                std::string buf;
                auto status = util::MessageToJsonString(message, &buf);
                assert_true(status.ok());
                payload[name] = buf;
            }


        };
        using ManagerType = Manager;
    };

    using JSONContextPtr = ContextPtr<JSONContextPolicy>;


    static JSONContextPtr CreateJSONContext(const nlohmann::json& json_object = {}) {
        return std::make_shared<IContext<JSONContextPolicy>>(json_object);
    }

    static JSONContextPtr CreateJSONContextWithString(const std::string& json_string = "") {
        nlohmann::json json_obj = json_string.empty() ?
                nlohmann::json {} :
                nlohmann::json::parse(json_string);
        return std::make_shared<IContext<JSONContextPolicy>>(json_obj);
    }

    static std::vector<JSONContextPtr> CreateBatchJSONContextWithString(const std::string& json_string = "") {
        nlohmann::json json_obj = json_string.empty() ?
                                  nlohmann::json::parse("[]") :
                                  nlohmann::json::parse(json_string);
        if (!json_obj.is_array()) {
            json_obj = nlohmann::json { json_obj };
        }
        std::vector<JSONContextPtr> result;
        result.reserve(json_obj.size());
        for(auto& item: json_obj) {
            result.push_back(CreateJSONContext(item));
        }
        return result;
    }

    static std::string DumpJSONContext(const JSONContextPtr& context) {
        return context->GetPayload().dump();

    }
}


#endif //INSTINCT_JSONCONTEXTPOLICY_HPP
