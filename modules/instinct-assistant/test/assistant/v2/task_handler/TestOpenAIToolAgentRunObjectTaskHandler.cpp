//
// Created by RobinQu on 2024/5/4.
//
#include <gtest/gtest.h>

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
        // create assistant
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("gpt3.5-turbo");
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
        ASSERT_TRUE(state1);
        ASSERT_EQ(state1->input().chat().messages(0).content(), prompt_line);
        ASSERT_EQ(state1->previous_steps_size(), 0);
    }



}
