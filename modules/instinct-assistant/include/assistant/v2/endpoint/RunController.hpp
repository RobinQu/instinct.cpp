//
// Created by RobinQu on 2024/4/19.
//

#ifndef RUNCONTROLLER_HPP
#define RUNCONTROLLER_HPP

#include "BaseController.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class RunController final: public BaseController {
    public:
        explicit RunController(const AssistantFacade &facade)
            : BaseController(facade) {
        }

        void Mount(HttpLibServer &server) override {
            server.PostRoute<CreateRunRequest, RunObject>("/v1/threads/:thread_id/runs", [&](CreateRunRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                const auto resp = facade_.run->CreateRun(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Run object is not retrieved after creation", 500);
                }
            });

            server.GetRoute<ListRunsRequest, ListRunsResponse>("/v1/threads/:thread_id/runs", [&](ListRunsRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                const auto resp = facade_.run->ListRuns(req);
                session.Respond(resp);
            });


            server.GetRoute<GetRunRequest, RunObject>("/v1/threads/:thread_id/runs/:run_id", [&](GetRunRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                const auto resp = facade_.run->RetrieveRun(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond(fmt::format("Run object canno be found with run_id: {}", req.run_id()));
                }
            });


            server.PostRoute<ModifyRunRequest, RunObject>("/v1/threads/:thread_id/runs/:run_id", [&](ModifyRunRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                const auto resp = facade_.run->ModifyRun(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Run object cannot be retireved after modification", 500);
                }
            });

            server.PostRoute<SubmitToolOutputsToRunRequest, RunObject>("/v1/threads/:thread_id/runs/:run_id/submit_tool_outputs", [&](SubmitToolOutputsToRunRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                const auto resp = facade_.run->SubmitToolOutputs(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Run object cannot be retrieved after tool outputs are submitted", 500);
                }
            });

            server.PostRoute<CancelRunRequest, RunObject>("/v1/threads/:thread_id/runs/:run_id/cancel", [&](CancelRunRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                const auto resp = facade_.run->CancelRun(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond("Run object cannot be retireved after cancelled", 500);
                }
            });

        }
    };
}



#endif //RUNCONTROLLER_HPP
