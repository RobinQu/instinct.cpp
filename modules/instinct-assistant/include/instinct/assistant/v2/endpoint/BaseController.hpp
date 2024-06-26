//
// Created by RobinQu on 2024/4/19.
//

#ifndef ASSISTANTAPIBASECONTROLLER_HPP
#define ASSISTANTAPIBASECONTROLLER_HPP

#include <utility>

#include <instinct/AssistantGlobals.hpp>
#include <instinct/assistant/v2/service/AssistantFacade.hpp>
#include <instinct/server/httplib/HttpLibServer.hpp>

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
