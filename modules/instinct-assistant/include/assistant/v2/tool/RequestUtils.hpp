//
// Created by RobinQu on 2024/5/6.
//

#ifndef REQUESTUTILS_HPP
#define REQUESTUTILS_HPP

#include <httplib.h>

#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class RequestUtils final {

    public:
        /**
         * Read query parameters from request and update to message with conventions
         * @param request
         * @param message
         * @return
         */
        static void LoadPaginationParameters(const httplib::Request& request, google::protobuf::Message& message) {
            auto* descriptor = message.GetDescriptor();
            auto* reflection = message.GetReflection();

            if (request.has_param("limit") && StringUtils::IsNotBlankString(request.get_param_value("limit"))) {

                if(auto* limit_field = descriptor->FindFieldByName("limit")) {
                    const auto limit = std::stoi(request.get_param_value("limit"));
                    reflection->SetInt32(&message, limit_field, limit);
                }
            }
            if (request.has_param("order") && StringUtils::IsNotBlankString(request.get_param_value("order"))) {
                if (auto *order_field = descriptor->FindFieldByName("order")) {
                    auto order = request.get_param_value("order");
                    ListRequestOrder order_value;
                    ListRequestOrder_Parse(order, &order_value);
                    reflection->SetEnumValue(&message, order_field, order_value);
                }
            }
            if (request.has_param("after") && StringUtils::IsNotBlankString(request.get_param_value("after"))) {
                if (auto *after_field = descriptor->FindFieldByName("after")) {
                    reflection->SetString(&message, after_field, request.get_param_value("after"));
                }
            }
            if (request.has_param("before") && StringUtils::IsNotBlankString(request.get_param_value("before"))) {
                if (auto *after_field = descriptor->FindFieldByName("before")) {
                    reflection->SetString(&message, after_field, request.get_param_value("before"));
                }
            }
        }
    };

}


#endif //REQUESTUTILS_HPP
