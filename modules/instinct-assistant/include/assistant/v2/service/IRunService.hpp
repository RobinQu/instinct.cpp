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

        virtual RunObject CreateThreadAndRun(const CreateThreadAndRunRequest& create_thread_and_run_request) = 0;
        virtual RunObject CreateRun(const CreateRunRequest& create_request) = 0;
        virtual ListRunsResponse ListRuns(const ListRunsRequest& list_request) = 0;
        virtual RunObject RetrieveRun(const GetRunRequest& get_request) = 0;
        virtual RunObject ModifyRun(const ModifyRunRequest& modify_run_request) = 0;
        virtual RunObject SubmitToolOutputs(const SubmitToolOutputsToRunRequest& sub_request) = 0;
        virtual RunObject CancelRun(const CancelRunRequest& cancel_request) = 0;
        virtual ListRunStepsResponse ListRunSteps(const ListRunStepsRequest& list_run_steps_request) = 0;
        virtual RunObject GetRunStep(const GetRunStepRequest& get_run_step_request) = 0;
    };

    using IRunServicePtr = std::shared_ptr<IRunService>;

}

#endif //IRUNSERVICE_HPP
