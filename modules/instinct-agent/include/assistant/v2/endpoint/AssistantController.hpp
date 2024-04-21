//
// Created by RobinQu on 2024/4/19.
//

#ifndef ASSISTANTCONTROLLER_HPP
#define ASSISTANTCONTROLLER_HPP

#include "BaseController.hpp"
#include "assistant/v2/service/AssistantFacade.hpp"
#include "server/httplib/HttpLibServer.hpp"


namespace INSTINCT_AGENT_NS::assistant::v2 {
    using namespace INSTINCT_SERVER_NS;

    class AssistantController final: public BaseController {
    public:
        explicit AssistantController(const AssistantFacade &facade)
            : BaseController(facade) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetHttpLibServer().Post("/assitants", [&](const Request& req, Response& resp) {
                const auto req_entity = ProtobufUtils::Deserialize<ListAssistantsRequest>(req.body);
                const auto response_entity = facade_.assistant->ListAssistants(req_entity);
                resp.body = ProtobufUtils::Serialize(response_entity);
            });
        }
    };
}


#endif //ASSISTANTCONTROLLER_HPP
