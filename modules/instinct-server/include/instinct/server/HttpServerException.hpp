//
// Created by RobinQu on 2024/4/23.
//

#ifndef HTTPSERVEREXCEPTION_HPP
#define HTTPSERVEREXCEPTION_HPP

#include <utility>

#include <instinct/CoreGlobals.hpp>
#include <instinct/ServerGlobals.hpp>
#include <instinct/exception/InstinctException.hpp>


namespace INSTINCT_SERVER_NS {
    using namespace INSTINCT_CORE_NS;

    class HttpServerException final: public InstinctException {


    public:
        int status_code;
        std::string type;
        nlohmann::json param;
        std::string code;
        std::string message;

        explicit HttpServerException(
            const std::string& message,
            const int status_code = 400,
            std::string type = "invalid_request_error",
            std::string code = "",
            nlohmann::json param = {}
            )
            :   InstinctException(message),
                status_code(status_code),
                type(std::move(type)),
                param(std::move(param)),
                code(std::move(code)) {
        }
    };


}



#endif //HTTPSERVEREXCEPTION_HPP
