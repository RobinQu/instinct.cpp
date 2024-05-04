//
// Created by RobinQu on 2024/4/23.
//
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include "AssistantTestGlobals.hpp"


namespace INSTINCT_ASSISTANT_NS {

    class AssistantServiceTest: public BaseAssistantApiTest {

    };

    TEST_F(AssistantServiceTest, SimpleCRUD) {
        auto assistant_service = CreateAssistantService();

        // create
        AssistantObject create_request;
        create_request.set_model("ollama/mixtral:latest");
        auto object = assistant_service->CreateAssistant(create_request);
        LOG_INFO("obj: {}", object->ShortDebugString());
        ASSERT_EQ(object->model(), "ollama/mixtral:latest");

        // reteive
        GetAssistantRequest get_assistant_request;
        get_assistant_request.set_assistant_id(object->id());
        auto object2 = assistant_service->RetrieveAssistant(get_assistant_request);
        auto diff = util::MessageDifferencer {};
        ASSERT_TRUE(diff.Compare(object.value(), object2.value()));

        // update
        ModifyAssistantRequest modify_assistant_request;
        modify_assistant_request.set_assistant_id(object->id());
        google::protobuf::Value value;
        value.set_string_value("world");
        modify_assistant_request.mutable_metadata()->mutable_fields()->emplace("hello", value);
        modify_assistant_request.set_instructions("Be brave!");
        modify_assistant_request.set_temperature(0.1);
        auto object3 = assistant_service->ModifyAssistant(modify_assistant_request);
        ASSERT_EQ(ProtobufUtils::Serialize(object3->metadata()), R"({"hello":"world"})");
        // ASSERT_EQ(modify_assistant_request.temperature(), 0.1);
        ASSERT_EQ(modify_assistant_request.instructions(), "Be brave!");


        // create again
        AssistantObject create_request2;
        create_request.set_model("ollama/llama2:latest");
        auto object5 = assistant_service->CreateAssistant(create_request);
        ASSERT_TRUE(object5.has_value());
        ASSERT_EQ(object5->model(), "ollama/llama2:latest");

        // list all
        ListAssistantsRequest list_assistants_request;
        auto object6 = assistant_service->ListAssistants(list_assistants_request);
        LOG_INFO("list_resp: {}", object6.ShortDebugString());
        ASSERT_FALSE(object6.has_more());
        ASSERT_EQ(object6.first_id(), object5->id());
        ASSERT_EQ(object6.last_id(), object->id());
        ASSERT_EQ(object6.data_size(), 2);
        ASSERT_EQ(object6.object(), "list");
        ASSERT_EQ(object6.data(1).id(), object->id());
        ASSERT_EQ(object6.data(0).id(), object5->id());

        // list with cursor
        list_assistants_request.set_after(object->id());
        auto object7 = assistant_service->ListAssistants(list_assistants_request);
        ASSERT_EQ(object7.data_size(), 1);
        ASSERT_TRUE(diff.Compare(object6.data(0), object5.value()));

        // delete
        DeleteAssistantRequest delete_assistant_request;
        delete_assistant_request.set_assistant_id(object->id());
        auto object4 = assistant_service->DeleteAssistant(delete_assistant_request);
        ASSERT_TRUE(object4.deleted());
        ASSERT_EQ(object4.id(), object->id());

        // list again
        ListAssistantsRequest list_assistants_request2;
        auto object8 = assistant_service->ListAssistants(list_assistants_request2);
        ASSERT_EQ(object8.data_size(), 1);
        ASSERT_TRUE(diff.Compare(object6.data(0), object5.value()));
    }

}