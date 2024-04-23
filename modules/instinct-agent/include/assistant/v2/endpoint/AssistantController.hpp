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
            server.GetRoute<ListAssistantsRequest, ListAssistantsResponse>("/assistants", [&](ListAssistantsRequest& req, const HttpLibSession& session) {
                return facade_.assistant->ListAssistants(req);
            });

            server.PostRoute<AssistantObject, AssistantObject>("/assitants", [&](AssistantObject& req, const HttpLibSession& session) {
                return facade_.assistant->CreateAssistant(req).value();
            });

            server.GetRoute<GetAssistantRequest, AssistantObject>("/assitants/:assitant_id", [&](GetAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                return facade_.assistant->RetrieveAssistant(req).value();
            });

            server.PostRoute<ModifyAssistantRequest, AssistantObject>("/assistants/:assistant_id", [&](ModifyAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                return facade_.assistant->ModifyAssistant(req).value();
            });

            server.DeleteRoute<DeleteAssistantRequest, DeleteAssistantResponse>("/assitants/:assistant:id", [&](DeleteAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                return facade_.assistant->DeleteAssistant(req);
            });
        }
    };
}


#endif //ASSISTANTCONTROLLER_HPP
