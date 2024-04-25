//
// Created by RobinQu on 2024/4/24.
//

#ifndef HTTPLIBSESSION_HPP
#define HTTPLIBSESSION_HPP
#include <httplib.h>

#include "ServerGlobals.hpp"
#include "server/HttpServerException.hpp"
#include "tools/ProtobufUtils.hpp"

namespace INSTINCT_SERVER_NS {
    using namespace httplib;
    using namespace INSTINCT_CORE_NS;

    struct HttpLibSession {
        const Request& request;
        Response& response;

        template<typename Res, typename EntityConverter=ProtobufUtils>
        requires IsProtobufMessage<Res>
        void Respond(const Res& resp_entity, const int code = 200, const HttpHeaders& headers = {}) const {
             EntityConverter::Serialize(resp_entity, response.body);
            response.status = code;
            for(const auto& [k,v]: headers) {
                response.set_header(k,v);
            }
        }

        void Respond(
            const std::string& message,
            const int status_code = 400,
            const std::string& type = "invliad_request_error",
            const std::string& code = "",
            const nlohmann::json& param = {}) const {
            response.status = status_code;
            nlohmann::json body;
            body["error"]["message"] = message;
            body["error"]["type"] = type;
            body["error"]["param"] = param;
            body["error"]["code"] = code;
            response.body = body.dump();
        }

        void Respond(const HttpServerException& exception) const {
            Respond(exception.message, exception.status_code, exception.type, exception.code, exception.param);
        }


    };
}

#endif //HTTPLIBSESSION_HPP
