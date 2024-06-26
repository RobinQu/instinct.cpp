//
// Created by RobinQu on 2024/4/25.
//
#include <gtest/gtest.h>
#include <instinct/assistant_test_global.hpp>
#include <instinct/assistant/v2/service/thread_service.hpp>

namespace INSTINCT_ASSISTANT_NS {
    class ThreadServiceTest: public BaseAssistantApiTest {

    };

    TEST_F(ThreadServiceTest, SimpleCRUD) {
        const auto thread_service = CreateThreadService();
        const auto message_service = CreateMessageService();

        // insert one thread without messages
        ThreadObject create_request1;
        create_request1.mutable_tool_resources()->mutable_file_search()->add_vector_store_ids("vs-1234");
        auto obj1 = thread_service->CreateThread(create_request1);
        LOG_INFO("insert one returned: {}", obj1->ShortDebugString());

        // insert one thread with two messages
        ThreadObject create_request2;
        const auto msg1 = create_request2.add_messages();
        // auto* content = msg1->add_content();
        // content->set_type(MessageObject_MessageContentType_text);
        msg1->set_content("Why sky is blue?");
        msg1->set_role(user);
        const auto msg2 = create_request2.add_messages();
        // auto* content2 = msg2->add_content();
        // content2->set_type(MessageObject_MessageContentType_text);
        msg2->set_content(R"( The sky appears blue because of a process called Rayleigh scattering. As sunlight reaches Earth's atmosphere, it is made up of different colors, which are represented in the light spectrum as having different wavelengths. Blue and violet light have the shortest wavelengths and are scattered in all directions by the gas molecules in the earth's atmosphere.

This scattering of light is what makes the sky look blue in our perception. You might wonder why we don't see a violet sky, given that violet light is scattered more than blue light. This is because our eyes are more sensitive to blue light and because sunlight reaches us with less violet light to begin with. Additionally, some of the violet light gets absorbed by the ozone layer in the atmosphere. As a result, when we look at the sky, we perceive it as blue, not violet.)");
        msg2->set_role(assistant);
        auto obj2 = thread_service->CreateThread(create_request2);
        LOG_INFO("insert one returned: {}", obj2->ShortDebugString());
        ListMessagesRequest list_messages_request;
        list_messages_request.set_thread_id(obj2->id());
        list_messages_request.set_order(desc);
        const auto obj6 = message_service->ListMessages(list_messages_request);
        auto& msg2_in_list = obj6.data(1);
        ASSERT_EQ(msg2_in_list.content(0).text().value(), msg2->content());

        // get thread
        GetThreadRequest get_thread_request;
        get_thread_request.set_thread_id(obj1->id());
        auto obj3 = thread_service->RetrieveThread(get_thread_request);
        LOG_INFO("get one returned: {}", obj3->ShortDebugString());
        ASSERT_GT(obj3->created_at(), 0);
        ASSERT_GT(obj3->modified_at(), 0);
        ASSERT_EQ(obj3->object(), "thread");
        ASSERT_EQ(obj3->id(), obj1->id());
        ASSERT_EQ(obj3->metadata().fields_size(), 0);
        ASSERT_EQ(obj3->has_tool_resources(), true);

        // update thread
        ModifyThreadRequest modify_thread_request;
        modify_thread_request.set_thread_id(obj1->id());
        google::protobuf::Value string_v;
        string_v.set_string_value("bar");
        modify_thread_request.mutable_metadata()->mutable_fields()->emplace("foo", string_v);
        auto obj4 = thread_service->ModifyThread(modify_thread_request);
        LOG_INFO("update one returned: {}", obj4->ShortDebugString());
        ASSERT_EQ(obj4->metadata().fields_size(), 1);
        ASSERT_EQ(obj4->metadata().fields().at("foo").string_value(), "bar");

        // delete thread and check again
        DeleteThreadRequest delete_thread_request;
        delete_thread_request.set_thread_id(obj1->id());
        auto delete_response = thread_service->DeleteThread(delete_thread_request);
        LOG_INFO("delete one returned: {}", delete_response.ShortDebugString());
        ASSERT_TRUE(delete_response.deleted());
        ASSERT_EQ(delete_response.object(), "thread.deleted");
        ASSERT_EQ(delete_response.id(), obj1->id());
        auto obj5 = thread_service->RetrieveThread(get_thread_request);
        ASSERT_FALSE(obj5.has_value());
    }
}
