//
// Created by RobinQu on 2024/4/19.
//

#ifndef ASSISTANTCONTROLLER_HPP
#define ASSISTANTCONTROLLER_HPP

#include <instinct/assistant/v2/endpoint/BaseController.hpp>
#include <instinct/assistant/v2/service/AssistantFacade.hpp>
#include <instinct/assistant/v2/tool/RequestUtils.hpp>
#include <instinct/server/httplib/http_lib_server.hpp>


namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_SERVER_NS;

    class AssistantController final: public BaseController {
    public:
        explicit AssistantController(const AssistantFacade &facade)
            : BaseController(facade) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetRoute<ListAssistantsRequest, ListAssistantsResponse>("/v1/assistants", [&](ListAssistantsRequest& req, const HttpLibSession& session) {
                RequestUtils::LoadPaginationParameters(session.request, req);
                session.Respond(facade_.assistant->ListAssistants(req));
            });

            server.PostRoute<AssistantObject, AssistantObject>("/v1/assistants", [&](const AssistantObject& req, const HttpLibSession& session) {
                auto v = facade_.assistant->CreateAssistant(req);
                if (v.has_value()) {
                    session.Respond(v.value());
                } else {
                    session.Respond("Assistant is not retrieved after createion", 500);
                }
            });

            server.GetRoute<GetAssistantRequest, AssistantObject>("/v1/assistants/:assitant_id", [&](GetAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                const auto resp = facade_.assistant->RetrieveAssistant(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Assistant object cannot be retrieved after creation", 500);
                }
            });

            server.PostRoute<ModifyAssistantRequest, AssistantObject>("/v1/assistants/:assistant_id", [&](ModifyAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                const auto resp = facade_.assistant->ModifyAssistant(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Assistant object cannot be retrieved after modification", 500);
                }
            });

            server.DeleteRoute<DeleteAssistantRequest, DeleteAssistantResponse>("/v1/assistants/:assistant_id", [&](DeleteAssistantRequest& req, const HttpLibSession& session) {
                req.set_assistant_id(session.request.path_params.at("assistant_id"));
                session.Respond(facade_.assistant->DeleteAssistant(req));
            });
        }
    };
}


#endif //ASSISTANTCONTROLLER_HPP
