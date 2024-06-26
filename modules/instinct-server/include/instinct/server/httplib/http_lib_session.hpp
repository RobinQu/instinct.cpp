//
// Created by RobinQu on 2024/4/24.
//

#ifndef HTTPLIBSESSION_HPP
#define HTTPLIBSESSION_HPP
#include <httplib.h>

#include <instinct/server_global.hpp>
#include <instinct/server/http_server_exception.hpp>
#include <instinct/tools/protobuf_utils.hpp>

namespace INSTINCT_SERVER_NS {
    using namespace httplib;
    using namespace INSTINCT_CORE_NS;

    struct HttpLibSession {
        const Request& request;
        Response& response;

        static nlohmann::json CreateErrorObject(const std::string& message,
            const int status_code,
            const std::string& type = "invliad_request_error",
            const std::string& code = "",
            const nlohmann::json& param = {}) {
            nlohmann::json body;
            body["error"]["message"] = message;
            body["error"]["type"] = type;
            body["error"]["param"] = param;
            body["error"]["code"] = code;
            return body;
        }

        static void Respond(Response& response,
            const std::string& message,
            const int status_code,
            const std::string& type = "invliad_request_error",
            const std::string& code = "",
            const nlohmann::json& param = {}
            ) {
            response.status = status_code;
            const auto body = CreateErrorObject(message, status_code, type, code, param);
            response.body = body.dump();
        }

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
            const std::string& message
            ) const {
            response.status = 200;
            response.body = message;
        }

        void Respond(
            const std::string& message,
            const int status_code,
            const std::string& type = "invliad_request_error",
            const std::string& code = "",
            const nlohmann::json& param = {}) const {
            HttpLibSession::Respond(response, message, status_code, type, code, param);
        }

        void Respond(const HttpServerException& exception) const {
            Respond(exception.message, exception.status_code, exception.type, exception.code, exception.param);
        }


    };
}

#endif //HTTPLIBSESSION_HPP
