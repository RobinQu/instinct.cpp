//
// Created by RobinQu on 2024/5/4.
//
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

#include <instinct/assistant_test_global.hpp>
#include <instinct/llm_test_global.hpp>
#include <instinct/assistant/v2/service/run_service.hpp>
#include <instinct/assistant/v2/task_handler/run_object_task_handler.hpp>
#include <instinct/chat_model/openai_chat.hpp>
#include <instinct/toolkit/local_toolkit.hpp>


namespace INSTINCT_ASSISTANT_NS::v2 {

    class TestRunObjectTaskHandler: public BaseAssistantApiTest {
    protected:
        RunServicePtr run_service_ = CreateRunServiceWithoutScheduler();
        MessageServicePtr message_service_ = CreateMessageService();
        AssistantServicePtr assistant_service_ = CreateAssistantService();
        ThreadServicePtr thread_service_ = CreateThreadService();
        ChatModelPtr chat_model_ = CreateOpenAIChatModel({
            .temperature = 0
        });
        EmbeddingsPtr embedding_model_ = CreateOpenAIEmbeddingModel(
            // {
            //     .endpoint = {.host = "localhost", .port = 8000, .protocol = kHTTP},
            //     .dimension = 768
            // }
        );
        VectorStoreOperatorPtr vector_store_operator_ = CreateDuckDBStoreOperator(duck_db_, embedding_model_, vector_store_metadata_data_mapper_);
        RetrieverOperatorPtr retriever_operator_ = CreateSimpleRetrieverOperator(vector_store_operator_, duck_db_, {.table_name = "docs_" + ChronoUtils::GetCurrentTimestampString()});
        VectorStoreServicePtr vector_store_service_ = CreateVectorStoreService(nullptr, retriever_operator_);
        FileServicePtr file_service_ = CreateFileService();
        std::filesystem::path asset_dir_ = std::filesystem::current_path() / "_assets";
        SummaryChainPtr summary_chain = CreateSummaryChain(chat_model_);
        CitationAnnotatingChainPtr citation_annotating_chain = CreateCitationAnnotatingChain(chat_model_);
        FileObjectTaskHandler file_object_task_handler {retriever_operator_, vector_store_service_, file_service_, summary_chain};

        std::shared_ptr<RunObjectTaskHandler> CreateTaskHandler() {
            ModelProviderOptions llm_provider_options;
            LoadOpenAIChatConfiguration(llm_provider_options.openai);
            llm_provider_options.provider = ModelProvider::kOPENAI;

            AgentExecutorOptions agent_executor_options;
            return std::make_shared<RunObjectTaskHandler>(
                run_service_,
                message_service_,
                assistant_service_,
                retriever_operator_,
                vector_store_service_,
                thread_service_,
                citation_annotating_chain,
                llm_provider_options,
                agent_executor_options
            );
        }

        std::string UploadFileSearchResource_(const VectorStoreObject& vector_store_object, const std::filesystem::path& file_path) {
            // upload pdf file
            UploadFileRequest upload_file_request;
            upload_file_request.set_filename(file_path.filename());
            upload_file_request.set_purpose(FileObjectPurpose::assistants);
            std::fstream fstream  {file_path, std::ios::binary | std::ios::in};
            const auto file_object1 = file_service_->UploadFile(upload_file_request, fstream);
            fstream.close();
            // ASSERT_TRUE(file_object1);

            // create vs_file
            CreateVectorStoreFileRequest create_vector_store_file_request;
            create_vector_store_file_request.set_file_id(file_object1->id());
            create_vector_store_file_request.set_vector_store_id(vector_store_object.id());
            const auto vector_store_file_object1 = vector_store_service_->CreateVectorStoreFile(create_vector_store_file_request);
            // ASSERT_TRUE(vector_store_file_object1);
            LOG_INFO("vector_store_file_object1={}", vector_store_file_object1->ShortDebugString());

            CommonTaskScheduler::Task task {
                .task_id = vector_store_file_object1->file_id(),
                .category =  FileObjectTaskHandler::CATEGORY,
                .payload = ProtobufUtils::Serialize(vector_store_file_object1.value())
            };

            // test handle
            file_object_task_handler.Handle(task);

            // validate vs_file
            GetVectorStoreFileRequest get_vector_store_file_request;
            get_vector_store_file_request.set_vector_store_id(vector_store_object.id());
            get_vector_store_file_request.set_file_id(vector_store_file_object1->file_id());
            const auto vector_store_file2 = vector_store_service_->GetVectorStoreFile(get_vector_store_file_request);
            LOG_INFO("vector_store_file2={}", vector_store_file2->ShortDebugString());
            assert_true(vector_store_file2->status() == completed);

            return file_object1->id();
        }
    };

    TEST_F(TestRunObjectTaskHandler, RecoverAgentStateWithSuccessfulSteps) {
        google::protobuf::util::MessageDifferencer diff;

        // create assistant
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("gpt-3.5-turbo");
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
        // auto* content = msg->add_content();
        // content->mutable_text()->set_value(prompt_line);
        // content->set_type(MessageObject_MessageContentType_text);
        msg->set_content(prompt_line);
        const auto obj2 = run_service_->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create handler
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
        modify_run_request.set_status(RunObject_RunObjectStatus_queued);
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

        ASSERT_TRUE(continuation_step.thought().continuation().has_tool_call_message());
        ASSERT_EQ(continuation_step.thought().continuation().tool_call_message().tool_calls_size(), 1);
        const auto& call1 = continuation_step.thought().continuation().tool_call_message().tool_calls(0);
        ASSERT_EQ(call1.id(), "call-1");
        ASSERT_EQ(call1.function().name(), "foo");
        ASSERT_EQ(call1.function().arguments(), "{}");
        const auto& pause_step = state2->previous_steps(1);
        ASSERT_TRUE(pause_step.has_thought());
        ASSERT_TRUE(pause_step.thought().has_pause());
        ASSERT_EQ(pause_step.thought().pause().completed_size(), 0);
        ASSERT_EQ(pause_step.thought().pause().tool_call_message().tool_calls_size(), 1);
        ASSERT_TRUE(diff.Compare(pause_step.thought().pause().tool_call_message(), continuation_step.thought().continuation().tool_call_message()));


        // expect to get observation step
        // 1. fill output to step_details to mock generation of observation
        ModifyRunStepRequest modify_run_step_request;
        tool_call->mutable_function()->set_output("bar");
        modify_run_step_request.set_step_id(create_run_step_response->id());
        modify_run_step_request.set_thread_id(obj2->thread_id());
        modify_run_step_request.set_run_id(obj2->id());
        modify_run_step_request.mutable_step_details()->mutable_tool_calls()->Add()->CopyFrom(*tool_call);
        modify_run_step_request.set_status(RunStepObject_RunStepStatus_completed);
        const auto obj4 = run_service_->ModifyRunStep(modify_run_step_request);
        ASSERT_TRUE(obj4);
        // 2. mock run object
        modify_run_request.set_status(RunObject_RunObjectStatus_in_progress);
        ASSERT_TRUE(run_service_->ModifyRun(modify_run_request));
        // 3. assertions
        const auto state3 = task_handler->RecoverAgentState(obj3.value());
        LOG_INFO("RecoverAgentState returned: {}", state3->ShortDebugString());
        ASSERT_EQ(state3->previous_steps_size(), 2); // only one continuation and one observation. pause is lost as it's intermediate state.
        ASSERT_TRUE(diff.Compare(state3->previous_steps(0), state2->previous_steps(0)));
        auto& observation_step = state3->previous_steps(1);
        ASSERT_TRUE(observation_step.has_observation());
        ASSERT_EQ(observation_step.observation().tool_messages_size(), 1);
        ASSERT_EQ(observation_step.observation().tool_messages(0).content(), "bar");

        // expect to get agent finish with successful response
        // 1. mock message
        CreateMessageRequest create_message_request;
        create_message_request.set_thread_id(obj3->thread_id());
        create_message_request.set_content("hello!");
        create_message_request.set_role(assistant);
        const auto obj7 = message_service_->CreateMessage(create_message_request);
        ASSERT_TRUE(obj7);
        // 2. mock run step
        RunStepObject create_run_step_request2;
        create_run_step_request2.set_status(RunStepObject_RunStepStatus_completed);
        create_run_step_request2.set_run_id(obj2->id());
        create_run_step_request2.set_thread_id(obj2->thread_id());
        create_run_step_request2.set_type(RunStepObject_RunStepType_message_creation);
        create_run_step_request2.mutable_step_details()->mutable_message_creation()->set_message_id(obj7->id());
        const auto& obj6 = run_service_->CreateRunStep(create_run_step_request2);
        ASSERT_TRUE(obj6);
        // 3. mock run object
        modify_run_request.set_status(RunObject_RunObjectStatus_completed);
        const auto obj5 = run_service_->ModifyRun(modify_run_request);
        ASSERT_TRUE(obj5);
        // 3. assertions
        const auto state4 = task_handler->RecoverAgentState(obj5.value());
        LOG_INFO("RecoverAgentState returned: {}", state4->ShortDebugString());
        ASSERT_EQ(state4->previous_steps_size(), 3); // only one continuation and one observation. pause is lost as it's intermediate state.
        ASSERT_TRUE(diff.Compare(state4->previous_steps(0), state3->previous_steps(0)));
        ASSERT_TRUE(diff.Compare(state4->previous_steps(1), state3->previous_steps(1)));
        auto& finish_step = state4->previous_steps(2);
        ASSERT_TRUE(finish_step.has_thought());
        ASSERT_TRUE(finish_step.thought().has_finish());
        ASSERT_EQ(finish_step.thought().finish().response(), "hello!");

    }

    TEST_F(TestRunObjectTaskHandler, RecoverAgentStateWithFailedSteps) {
        google::protobuf::util::MessageDifferencer message_differencer;

        // create assistant
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("gpt-3.5-turbo");
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
        // auto* content = msg->add_content();
        // content->mutable_text()->set_value(prompt_line);
        // content->set_type(MessageObject_MessageContentType_text);
        msg->set_content(prompt_line);
        const auto obj2 = run_service_->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create handler
        const auto task_handler = CreateTaskHandler();

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

        // expect finish with cancellation
        ModifyRunStepRequest modify_run_step_request;
        modify_run_step_request.set_run_id(obj3->id());
        modify_run_step_request.set_thread_id(obj3->thread_id());
        modify_run_step_request.set_status(RunStepObject_RunStepStatus_cancelled);
        modify_run_step_request.set_step_id(create_run_step_response->id());
        ASSERT_TRUE(run_service_->ModifyRunStep(modify_run_step_request));
        modify_run_request.set_status(RunObject_RunObjectStatus_cancelled);
        const auto obj4 = run_service_->ModifyRun(modify_run_request);
        ASSERT_TRUE(obj4);
        const auto state4 = task_handler->RecoverAgentState(obj4.value());
        LOG_INFO("RecoverAgentState returned: {}", state4->ShortDebugString());
        ASSERT_EQ(state4->previous_steps_size(), 2);
        const auto& finish_step = state4->previous_steps(1).thought().finish();
        ASSERT_TRUE(finish_step.is_cancelled());

        // expect finish with expiration
        modify_run_request.set_status(RunObject_RunObjectStatus_expired);
        const auto obj5 = run_service_->ModifyRun(modify_run_request);
        ASSERT_TRUE(obj5);
        modify_run_step_request.set_status(RunStepObject_RunStepStatus_expired);
        ASSERT_TRUE(run_service_->ModifyRunStep(modify_run_step_request));
        const auto state5 = task_handler->RecoverAgentState(obj5.value());
        LOG_INFO("RecoverAgentState returned: {}", state5->ShortDebugString());
        ASSERT_EQ(state5->previous_steps_size(), 2);
        const auto& finish_step2 = state5->previous_steps(1).thought().finish();
        ASSERT_TRUE(finish_step2.is_expired());

        // expect finish with failure
        modify_run_request.set_status(RunObject_RunObjectStatus_failed);
        const auto obj6 = run_service_->ModifyRun(modify_run_request);
        ASSERT_TRUE(obj6);
        modify_run_step_request.set_status(RunStepObject_RunStepStatus_failed);
        ASSERT_TRUE(run_service_->ModifyRunStep(modify_run_step_request));
        const auto state6 = task_handler->RecoverAgentState(obj6.value());
        LOG_INFO("RecoverAgentState returned: {}", state6->ShortDebugString());
        ASSERT_EQ(state6->previous_steps_size(), 2);
        const auto& finish_step3 = state6->previous_steps(1).thought().finish();
        ASSERT_TRUE(finish_step3.is_failed());
    }

    TEST_F(TestRunObjectTaskHandler, TaskWithFunctionCalls) {
        // create tools
        FunctionToolkitPtr tool_kit = CreateLocalToolkit(
            std::make_shared<GetFlightPriceTool>(),
            std::make_shared<GetNightlyHotelPrice>(),
            // need a new chat model instance without tools
            std::make_shared<LLMMath>(CreateOpenAIChatModel())
        );

        // create assistant with tools
        AssistantObject create_assistant_request;
        create_assistant_request.set_model("gpt-3.5-turbo");
        create_assistant_request.set_temperature(0);
        for (const auto& tool_schema: tool_kit->GetAllFunctionToolSchema()) {
            auto* assistant_tool = create_assistant_request.mutable_tools()->Add();
            assistant_tool->set_type(function);
            assistant_tool->mutable_function()->CopyFrom(tool_schema);
        }
        const auto obj1 = assistant_service_->CreateAssistant(create_assistant_request);
        LOG_INFO("CreateAssistant returned: {}", obj1->ShortDebugString());
        ASSERT_EQ(obj1->tools_size(), tool_kit->GetAllFunctionToolSchema().size());

        // create thread and run
        const std::string prompt_line = "Which is cheapest ticket among those flights to New York, Paris, and Tokyo?";
        CreateThreadAndRunRequest create_thread_and_run_request1;
        create_thread_and_run_request1.set_assistant_id(obj1->id());
        auto* msg = create_thread_and_run_request1.mutable_thread()->add_messages();
        msg->set_role(user);
        // auto* content = msg->add_content();
        // content->mutable_text()->set_value(prompt_line);
        // content->set_type(MessageObject_MessageContentType_text);
        msg->set_content(prompt_line);
        const auto obj2 = run_service_->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create handler and task
        const auto task_handler = CreateTaskHandler();
        CommonTaskScheduler::Task task {
            .task_id = obj2->id(),
            .category =  RunObjectTaskHandler::CATEGORY,
            .payload = ProtobufUtils::Serialize(obj2.value())
        };

        // test accept
        ASSERT_TRUE(task_handler->Accept(task));

        // test handling
        task_handler->Handle(task);

        // assertions
        GetRunRequest get_run_request;
        get_run_request.set_thread_id(obj2->thread_id());
        get_run_request.set_run_id(obj2->id());
        auto obj3 = run_service_->RetrieveRun(get_run_request);
        ASSERT_EQ(obj3->status(), RunObject_RunObjectStatus_requires_action);

        while (obj3->status() == RunObject_RunObjectStatus_requires_action) {
            ListRunStepsRequest list_run_steps_request;
            list_run_steps_request.set_thread_id(obj2->thread_id());
            list_run_steps_request.set_run_id(obj2->id());
            list_run_steps_request.set_order(desc);
            const auto list_run_step_response = run_service_->ListRunSteps(list_run_steps_request);
            // ASSERT_GT(list_run_step_response.data_size(), 1);
            ASSERT_TRUE(list_run_step_response.data(0).step_details().tool_calls_size()>0);

            // submit function tool results
            SubmitToolOutputsToRunRequest submit_tool_outputs_to_run_request;
            submit_tool_outputs_to_run_request.set_thread_id(obj2->thread_id());
            submit_tool_outputs_to_run_request.set_run_id(obj3->id());
            submit_tool_outputs_to_run_request.set_stream(false);
            for(const auto& tool_call: list_run_step_response.data(0).step_details().tool_calls()) {
                ToolCallObject function_tool_invocation;
                function_tool_invocation.set_id(tool_call.id());
                function_tool_invocation.mutable_function()->set_name(tool_call.function().name());
                function_tool_invocation.mutable_function()->set_arguments(tool_call.function().arguments());
                FunctionToolResult function_tool_result = tool_kit->Invoke(function_tool_invocation);
                ASSERT_FALSE(function_tool_result.has_error());
                ASSERT_TRUE(StringUtils::IsNotBlankString(function_tool_result.return_value()));
                auto *output = submit_tool_outputs_to_run_request.mutable_tool_outputs()->Add();
                output->set_tool_call_id(tool_call.id());
                output->set_output(function_tool_result.return_value());
            }
            const auto obj4 = run_service_->SubmitToolOutputs(submit_tool_outputs_to_run_request);
            ASSERT_TRUE(obj4);
            // handle again with obj4
            task = {
                .task_id = obj2->id(),
                .category =  RunObjectTaskHandler::CATEGORY,
                .payload = ProtobufUtils::Serialize(obj4.value())
            };
            task_handler->Handle(task);

            // get latest run object
            obj3 = run_service_->RetrieveRun(get_run_request);
        }


        // assertions
        const auto obj5 = run_service_->RetrieveRun(get_run_request);
        ASSERT_EQ(obj5->status(), RunObject_RunObjectStatus_completed);
        ListRunStepsRequest list_run_steps_request;
        list_run_steps_request.set_thread_id(obj2->thread_id());
        list_run_steps_request.set_run_id(obj2->id());
        list_run_steps_request.set_order(desc);
        const auto list_run_step_response2 = run_service_->ListRunSteps(list_run_steps_request);
        ASSERT_GE(list_run_step_response2.data_size(), 2); // could be three or two
        // latest step must be result message
        ASSERT_TRUE(list_run_step_response2.data(0).step_details().has_message_creation());
        GetMessageRequest get_message_request;
        get_message_request.set_thread_id(obj5->thread_id());
        get_message_request.set_message_id(list_run_step_response2.data(0).step_details().message_creation().message_id());
        const auto obj6 = message_service_->RetrieveMessage(get_message_request);
        ASSERT_TRUE(obj6->content_size()>0);
        ASSERT_TRUE(obj6->content(0).has_text());
        LOG_INFO("final output: {}", obj6->content(0).text().value());
        ASSERT_TRUE(StringUtils::IsNotBlankString(obj6->content(0).text().value()));
    }

    TEST_F(TestRunObjectTaskHandler, TaskWithFileSearch) {
        // create vs
        CreateVectorStoreRequest create_vector_store_request;
        create_vector_store_request.set_name("test-vs");
        const auto vector_store_object1 = vector_store_service_->CreateVectorStore(create_vector_store_request);
        // ASSERT_TRUE(vector_store_object1);
        LOG_INFO("vector_store_object1={}", vector_store_object1->ShortDebugString());

        // add file
        const auto file_id_1 = UploadFileSearchResource_(vector_store_object1.value(), asset_dir_ / "Antimonumento 5J .txt");
        const auto file_id_2 = UploadFileSearchResource_(vector_store_object1.value(), asset_dir_ / "Double sovereign.txt");
        LOG_INFO("file ids: {}, {}", file_id_1, file_id_2);

        AssistantObject create_assistant_request;
        create_assistant_request.add_tools()->set_type(file_search);
        create_assistant_request.mutable_tool_resources()
            ->mutable_file_search()
            ->add_vector_store_ids(vector_store_object1->id());
        create_assistant_request.set_model("gpt-3.5-turbo");

        const auto obj1 = assistant_service_->CreateAssistant(create_assistant_request);
        LOG_INFO("CreateAssistant returned: {}", obj1->ShortDebugString());

        // create thread and run
        const std::string prompt_line = "What makes coins of double sovereign special?";
        CreateThreadAndRunRequest create_thread_and_run_request1;
        create_thread_and_run_request1.set_assistant_id(obj1->id());
        auto* msg = create_thread_and_run_request1.mutable_thread()->add_messages();
        msg->set_role(user);
        msg->set_content(prompt_line);
        const auto obj2 = run_service_->CreateThreadAndRun(create_thread_and_run_request1);
        LOG_INFO("CreateThreadAndRun returned: {}", obj2->ShortDebugString());

        // create handler and task
        const auto task_handler = CreateTaskHandler();
        CommonTaskScheduler::Task task {
            .task_id = obj2->id(),
            .category =  RunObjectTaskHandler::CATEGORY,
            .payload = ProtobufUtils::Serialize(obj2.value())
        };

        // test handling
        task_handler->Handle(task);

        GetRunRequest get_run_request;
        get_run_request.set_thread_id(obj2->thread_id());
        get_run_request.set_run_id(obj2->id());
        auto obj3 = run_service_->RetrieveRun(get_run_request);
        ASSERT_EQ(obj3->status(), RunObject_RunObjectStatus_completed);




    }

}
