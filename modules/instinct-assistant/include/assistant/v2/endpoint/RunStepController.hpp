//
// Created by RobinQu on 2024/4/19.
//

#ifndef RUNSTEPCONTROLLER_HPP
#define RUNSTEPCONTROLLER_HPP

#include "BaseController.hpp"
#include "assistant/v2/tool/RequestUtils.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class RunStepController final: public BaseController {
    public:
        explicit RunStepController(const AssistantFacade &facade)
            : BaseController(facade) {
        }

        void Mount(HttpLibServer &server) override {
            server.GetRoute<ListRunStepsRequest, ListRunStepsResponse>("/v1/threads/:thread_id/runs/:run_id/steps", [&](ListRunStepsRequest& req, const HttpLibSession& session) {
                RequestUtils::LoadPaginationParameters(session.request, req);
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                const auto resp = facade_.run->ListRunSteps(req);
                session.Respond(resp);
            });

            server.GetRoute<GetRunStepRequest, RunStepObject>("/v1/threads/:thread_id/runs/:run_id/steps/:step_id", [&](GetRunStepRequest& req, const HttpLibSession& session) {
                req.set_thread_id(session.request.path_params.at("thread_id"));
                req.set_run_id(session.request.path_params.at("run_id"));
                req.set_step_id(session.request.path_params.at("step_id"));
                const auto resp = facade_.run->GetRunStep(req);
                if (resp.has_value()) {
                    session.Respond(resp.value());
                } else {
                    session.Respond(fmt::format("Run step object is not found with given thread_id({}), run_id({}) and step_id({}).", req.thread_id(), req.run_id(), req.step_id()));
                }
            });
        }
    };
}



#endif //RUNSTEPCONTROLLER_HPP
