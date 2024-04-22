//
// Created by RobinQu on 2024/4/22.
//
#include <assistant_api_v2.pb.h>
#include <gtest/gtest.h>

#include "CoreGlobals.hpp"
#include "tools/ProtobufUtils.hpp"


namespace INSTINCT_CORE_NS {
    TEST(ProtobufUtilsTest, ConvertJSONObjectToMessage) {
        agent::assistant::v2::Message message;
        message.set_assistant_id("assistant-123");
        message.set_id("msg-123");
        message.mutable_content()->set_type(agent::assistant::v2::Message_MessageContent_MessageContentType::Message_MessageContent_MessageContentType_Text);
        message.mutable_content()->mutable_text()->set_value("hahha");
        const auto json_string = ProtobufUtils::Serialize(message);

        nlohmann::json output;
        ProtobufUtils::ConvertMessageToJsonObject(message, output);
        LOG_INFO("converted json object: {}", output.dump());
        ASSERT_EQ(output.dump(), json_string);
    }
}
