//
// Created by RobinQu on 2024/4/19.
//

#ifndef IRUNSERVICE_HPP
#define IRUNSERVICE_HPP


#include <assistant_api_v2.pb.h>
#include "AssistantGlobals.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {

    class IRunService {
    public:
        IRunService()=default;
        virtual ~IRunService()=default;
        IRunService(IRunService&&)=delete;
        IRunService(const IRunService&)=delete;

        virtual std::optional<RunObject> CreateThreadAndRun(const CreateThreadAndRunRequest& create_thread_and_run_request) = 0;
        virtual std::optional<RunObject> CreateRun(const CreateRunRequest& create_request) = 0;
        virtual ListRunsResponse ListRuns(const ListRunsRequest& list_request) = 0;
        virtual std::optional<RunObject> RetrieveRun(const GetRunRequest& get_request) = 0;
        virtual std::optional<RunObject> ModifyRun(const ModifyRunRequest& modify_run_request) = 0;
        virtual std::optional<RunObject> SubmitToolOutputs(const SubmitToolOutputsToRunRequest& sub_request) = 0;
        virtual std::optional<RunObject> CancelRun(const CancelRunRequest& cancel_request) = 0;
        virtual ListRunStepsResponse ListRunSteps(const ListRunStepsRequest& list_run_steps_request) = 0;
        virtual std::optional<RunStepObject> GetRunStep(const GetRunStepRequest& get_run_step_request) = 0;

        virtual std::optional<RunStepObject> CreateRunStep(const RunStepObject& create_request) = 0;
        virtual std::optional<RunStepObject> ModifyRunStep(const ModifyRunStepRequest& modify_reequest) = 0;

    };

    using RunServicePtr = std::shared_ptr<IRunService>;

}

#endif //IRUNSERVICE_HPP
