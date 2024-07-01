//
// Created by RobinQu on 2024/4/19.
//

#ifndef ASSISTANTAPIBASECONTROLLER_HPP
#define ASSISTANTAPIBASECONTROLLER_HPP

#include <utility>

#include <instinct/assistant_global.hpp>
#include <instinct/assistant/v2/service/assistant_facade.hpp>
#include <instinct/server/httplib/http_lib_server.hpp>

namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_SERVER_NS;

    class BaseController: public HttpLibController {
    protected:
        AssistantFacade facade_;

    public:
        explicit BaseController(AssistantFacade facade)
            : facade_(std::move(facade)) {
        }
    };
}

#endif //ASSISTANTAPIBASECONTROLLER_HPP
