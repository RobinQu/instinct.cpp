//
// Created by RobinQu on 2024/5/4.
//
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/IRunService.hpp"
#include "assistant/v2/service/impl/RunServiceImpl.hpp"
#include "assistant/v2/task_handler/OpenAIToolAgentRunObjectTaskHandler.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "toolkit/LocalToolkit.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {

    class TestOpenAIToolAgentRunObjectTaskHandler: public BaseAssistantApiTest {
    protected:

        RunServicePtr run_service_ = CreateRunService();
        MessageServicePtr message_service_ = CreateMessageService();
        AssistantServicePtr assistant_service_ = CreateAssistantService();
        FunctionToolkitPtr builtin_toolkit_ = CreateLocalToolkit({});
        ChatModelPtr chat_model_ = CreateOpenAIChatModel();

        std::shared_ptr<OpenAIToolAgentRunObjectTaskHandler> CreateTaskHandler() {
            return std::make_shared<OpenAIToolAgentRunObjectTaskHandler>(
                run_service_,
                message_service_,
                assistant_service_,
                chat_model_,
                builtin_toolkit_
            );
        }
    };

    TEST_F(TestOpenAIToolAgentRunObjectTaskHandler, RecoverAgentState) {
        google::protobuf::util::MessageDifferencer message_differencer;

        // create assistant
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("gpt3.5-turbo");
        auto* tool1 = create_assistant_request.mutable_tools()->Add();
        tool1->set_type(function);
        tool1->mutable_function()->set_name("foo");
        tool1->mutable_function()->set_description("foo foo");
        const auto obj1 = assistant_service_->CreateAssistant(create_assistant_request);
        LOG_INFO("CreateAssistant returned: {}", obj1->ShortDebugString());

        // create thread and run
        const std::string prompt_line = "What's the population of India?";
        CreateThreadAndRunRequest create_thread_and_run_request1;
        create_thread_and_run_request1.set_assistant_id(obj1->id());
        auto* msg = create_thread_and_run_request1.mutable_thread()->add_messages();
        msg->set_role(user);
        msg->mutable_content()->mutable_text()->set_value(prompt_line);
        msg->mutable_content()->set_type(MessageObject_MessageContentType_text);
        const auto obj2 = run_service_->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create hanlder
        const auto task_handler = CreateTaskHandler();

        // recover from initial state
        const auto state1 = task_handler->RecoverAgentState(obj2.value());
        LOG_INFO("RecoverAgentState returned: {}", state1->ShortDebugString());
        ASSERT_TRUE(state1);
        ASSERT_EQ(state1->input().chat().messages(0).content(), prompt_line);
        ASSERT_EQ(state1->previous_steps_size(), 0);

        // create single step with one tool call but no output
        // 1.update run status to requires_action
        ModifyRunRequest modify_run_request;
        modify_run_request.set_run_id(obj2->id());
        modify_run_request.set_thread_id(obj2->thread_id());
        modify_run_request.set_status(RunObject_RunObjectStatus_requires_action);
        const auto obj3 = run_service_->ModifyRun(modify_run_request);
        ASSERT_TRUE(obj3);
        // 2. create run step with tool call
        RunStepObject create_run_step_request;
        create_run_step_request.set_status(RunStepObject_RunStepStatus_in_progress);
        create_run_step_request.set_run_id(obj2->id());
        create_run_step_request.set_thread_id(obj2->thread_id());
        create_run_step_request.set_type(RunStepObject_RunStepType_tool_calls);
        auto* tool_call = create_run_step_request.mutable_step_details()->add_tool_calls();
        tool_call->set_id("call-1");
        tool_call->mutable_function()->set_name("foo");
        tool_call->set_type(function);
        tool_call->mutable_function()->set_arguments("{}");
        const auto create_run_step_response = run_service_->CreateRunStep(create_run_step_request);
        ASSERT_TRUE(create_run_step_response);
        // 3. assertions
        const auto state2 = task_handler->RecoverAgentState(obj3.value());
        LOG_INFO("RecoverAgentState returned: {}", state2->ShortDebugString());
        ASSERT_EQ(state2->previous_steps_size(), 2);
        const auto& continuation_step = state2->previous_steps(0);
        ASSERT_TRUE(continuation_step.has_thought());
        ASSERT_TRUE(continuation_step.thought().has_continuation());
        ASSERT_TRUE(continuation_step.thought().continuation().has_openai());
        ASSERT_TRUE(continuation_step.thought().continuation().openai().has_tool_call_message());
        ASSERT_EQ(continuation_step.thought().continuation().openai().tool_call_message().tool_calls_size(), 1);
        const auto& call1 = continuation_step.thought().continuation().openai().tool_call_message().tool_calls(0);
        ASSERT_EQ(call1.id(), "call-1");
        ASSERT_EQ(call1.function().name(), "foo");
        ASSERT_EQ(call1.function().arguments(), "{}");
        const auto& pause_step = state2->previous_steps(1);
        ASSERT_TRUE(pause_step.has_thought());
        ASSERT_TRUE(pause_step.thought().has_pause());
        ASSERT_TRUE(pause_step.thought().pause().has_openai());
        ASSERT_EQ(pause_step.thought().pause().openai().completed_size(), 0);
        ASSERT_EQ(pause_step.thought().pause().openai().tool_call_message().tool_calls_size(), 1);
        ASSERT_TRUE(message_differencer.Compare(pause_step.thought().pause().openai().tool_call_message(), continuation_step.thought().continuation().openai().tool_call_message()));



    }



}
