//
// Created by RobinQu on 2024/4/25.
//
#include <gtest/gtest.h>
#include <google/protobuf/util/message_differencer.h>

#include "AssistantTestGlobals.hpp"
#include "assistant/v2/service/impl/MessageServiceImpl.hpp"

namespace INSTINCT_ASSISTANT_NS::v2 {
    class TestMessageService: public BaseAssistantApiTest {

    };

    TEST_F(TestMessageService, SimpleCRUD) {
        const auto service = CreateMessageService();
        google::protobuf::util::MessageDifferencer message_differencer;

        // create message
        CreateMessageRequest create_message_request;
        create_message_request.set_thread_id("123");
        create_message_request.set_role(MessageRole::user);
        create_message_request.set_content("hello world");

        const auto create_message_response = service->CreateMessage(create_message_request);
        ASSERT_TRUE(create_message_response);
        ASSERT_EQ(create_message_response->object(), "thread.message");
        ASSERT_EQ(create_message_response->thread_id(), "123");
        ASSERT_TRUE(StringUtils::IsNotBlankString(create_message_response->id()));

        // modify message
        ModifyMessageRequest modify_message_request;
        modify_message_request.set_message_id(create_message_response->id());
        modify_message_request.set_thread_id(create_message_response->thread_id());
        google::protobuf::Value value;
        value.set_bool_value(false);
        modify_message_request.mutable_metadata()->mutable_fields()->emplace("bool_value", value);
        const auto modify_message_response = service->ModifyMessage(modify_message_request);
        ASSERT_TRUE(modify_message_response);
        ASSERT_EQ(modify_message_response->id(), create_message_response->id());

        // get message
        GetMessageRequest get_message_request;
        get_message_request.set_thread_id(modify_message_response->thread_id());
        get_message_request.set_message_id(modify_message_response->id());
        const auto get_message_response = service->RetrieveMessage(get_message_request);
        ASSERT_TRUE(message_differencer.Compare(get_message_response.value(), modify_message_response.value()));

        // list mesasge object
        ListMessagesRequest list_messages_request;
        list_messages_request.set_thread_id(get_message_response->thread_id());
        const auto list_messages_respnse = service->ListMessages(list_messages_request);
        ASSERT_EQ(list_messages_respnse.object(), "list");
        ASSERT_EQ(list_messages_respnse.data_size(), 1);
        ASSERT_TRUE(message_differencer.Compare(list_messages_respnse.data(0), get_message_response.value()));
    }
}