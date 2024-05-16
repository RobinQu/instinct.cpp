//
// Created by RobinQu on 2024/4/25.
//

#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IRunService.hpp"
#include "assistant/v2/service/impl/RunServiceImpl.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    class RunServiceTest: public BaseAssistantApiTest {
    public:
        AssistantServicePtr assistant_service = CreateAssistantService();
    };

    TEST_F(RunServiceTest, SimpleCRUDWihtRunObjects) {
        util::MessageDifferencer message_differencer;
        const auto run_service = CreateRunServiceWithoutScheduler();

        // create assistant
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("ollama/llama3:latest");
        const auto obj1 = assistant_service->CreateAssistant(create_assistant_request);
        LOG_INFO("CreateAssistant returned: {}", obj1->ShortDebugString());

        // create thread and run
        CreateThreadAndRunRequest create_thread_and_run_request1;
        create_thread_and_run_request1.set_assistant_id(obj1->id());
        auto* msg = create_thread_and_run_request1.mutable_thread()->add_messages();
        msg->set_role(user);
        // auto* content = msg->add_content();
        // content->mutable_text()->set_value("What's the population of India?");
        msg->set_content("What's the population of India?");
        // content->set_type(MessageObject_MessageContentType_text);
        const auto obj2 = run_service->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create run
        CreateRunRequest create_run_request1;
        create_run_request1.set_assistant_id(obj1->id());
        create_run_request1.set_thread_id(obj2->thread_id());
        auto* msg2 = create_run_request1.mutable_additional_messages()->Add();
        msg2->set_role(user);
        msg2->set_content("How many planets in solar system?");
        // auto* content2 = msg2->add_content();
        // content2->mutable_text()->set_value("How many planets in solar system?");
        // content2->set_type(MessageObject_MessageContentType_text);
        const auto obj3 = run_service->CreateRun(create_run_request1);
        LOG_INFO("CreateRun returned: {}", obj3->ShortDebugString());

        // get run
        GetRunRequest get_run_request;
        get_run_request.set_run_id(obj2->id());
        get_run_request.set_thread_id(obj2->thread_id());
        const auto obj4 = run_service->RetrieveRun(get_run_request);
        LOG_INFO("RetrieveRun returned: {}", obj4->ShortDebugString());
        ASSERT_EQ(obj4->object(), "thread.run");
        ASSERT_EQ(obj4->status(), RunObject_RunObjectStatus_queued);
        ASSERT_GT(obj4->created_at(), 0);
        ASSERT_GT(obj4->modified_at(), 0);

        // list run
        ListRunsRequest list_runs_request1;
        list_runs_request1.set_thread_id(obj2->thread_id());
        list_runs_request1.set_order(desc);
        ListRunsResponse list_runs_response1 = run_service->ListRuns(list_runs_request1);
        LOG_INFO("ListRuns returned: {}", list_runs_response1.ShortDebugString());
        ASSERT_EQ(list_runs_response1.object(), "list");
        ASSERT_EQ(list_runs_response1.data_size(), 2);
        ASSERT_TRUE(message_differencer.Compare(list_runs_response1.data(0), obj3.value()));
        ASSERT_TRUE(message_differencer.Compare(list_runs_response1.data(1), obj2.value()));

        // update run
        ModifyRunRequest modify_run_request;
        modify_run_request.set_run_id(obj3->id());
        modify_run_request.set_thread_id(obj3->thread_id());
        google::protobuf::Value string_value;
        string_value.set_string_value("bar");
        modify_run_request.mutable_metadata()->mutable_fields()->emplace("foo", string_value);
        const auto obj5 = run_service->ModifyRun(modify_run_request);
        LOG_INFO("ModifyRun returned: {}", obj5->ShortDebugString());
        ASSERT_EQ(obj5->metadata().fields().at("foo").string_value(), "bar");

        // cancel run
        CancelRunRequest cancel_run_request;
        cancel_run_request.set_run_id(obj2->id());
        cancel_run_request.set_thread_id(obj2->thread_id());
        const auto obj6 = run_service->CancelRun(cancel_run_request);
        LOG_INFO("CancelRun returned: {}", obj6->ShortDebugString());
        ASSERT_EQ(obj6->status(), RunObject::RunObjectStatus::RunObject_RunObjectStatus_cancelling);
    }


    TEST_F(RunServiceTest, SimpleCRUDWithRunStepObjects) {
        const auto run_service = CreateRunServiceWithoutScheduler();
        google::protobuf::util::MessageDifferencer message_differencer;

        // create asssitant
        AssistantObject create_assitant_request;
        create_assitant_request.set_model("ollama/llama3:latest");
        const auto obj1 = assistant_service->CreateAssistant(create_assitant_request);
        LOG_INFO("CreateAssistant returned: {}", obj1->ShortDebugString());

        // create thread and run
        CreateThreadAndRunRequest create_thread_and_run_request1;
        create_thread_and_run_request1.set_assistant_id(obj1->id());
        auto* msg = create_thread_and_run_request1.mutable_thread()->add_messages();
        msg->set_role(user);
        // auto* content = msg->add_content();
        // content->mutable_text()->set_value("What's the population of India?");
        // content->set_type(MessageObject_MessageContentType_text);
        msg->set_content("What's the population of India?");
        const auto obj2 = run_service->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());


        // create run step
        RunStepObject create_run_step_request;
        create_run_step_request.set_run_id(obj2->id());
        create_run_step_request.set_thread_id(obj2->thread_id());
        create_run_step_request.set_assistant_id(obj2->assistant_id());
        create_run_step_request.set_type(RunStepObject_RunStepType_message_creation);
        create_run_step_request.set_status(RunStepObject_RunStepStatus_in_progress);
        create_run_step_request.mutable_step_details()->mutable_message_creation()->set_message_id("balbalba");
        const auto obj3 = run_service->CreateRunStep(create_run_step_request);
        LOG_INFO("CreateRunStep returned: {}", obj3->ShortDebugString());
        ASSERT_EQ(obj3->object(), "thread.run.step");
        ASSERT_TRUE(StringUtils::IsNotBlankString(obj3->id()));

        // modify run step
        ModifyRunStepRequest modify_run_step_request;
        modify_run_step_request.set_step_id(obj3->id());
        modify_run_step_request.set_thread_id(obj2->thread_id());
        modify_run_step_request.set_run_id(obj2->id());
        modify_run_step_request.set_status(RunStepObject_RunStepStatus_failed);
        const auto obj4 = run_service->ModifyRunStep(modify_run_step_request);
        LOG_INFO("ModifyRunStep returned: {}", obj4->ShortDebugString());
        ASSERT_EQ(obj4->status(), RunStepObject_RunStepStatus_failed);

        // get run step
        GetRunStepRequest get_run_step_request;
        get_run_step_request.set_step_id(obj3->id());
        get_run_step_request.set_thread_id(obj2->thread_id());
        get_run_step_request.set_run_id(obj2->id());
        const auto obj5 = run_service->GetRunStep(get_run_step_request);
        ASSERT_TRUE(message_differencer.Compare(obj5.value(), obj4.value()));

        // list run steps
        ListRunStepsRequest list_run_steps_request;
        list_run_steps_request.set_order(desc);
        list_run_steps_request.set_run_id(obj2->id());
        list_run_steps_request.set_thread_id(obj2->thread_id());
        const auto obj6 = run_service->ListRunSteps(list_run_steps_request);
        ASSERT_EQ(obj6.data_size(), 1);
        ASSERT_TRUE(message_differencer.Compare(obj6.data(0), obj5.value()));
    }
}
