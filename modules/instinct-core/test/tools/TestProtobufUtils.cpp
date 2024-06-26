//
// Created by RobinQu on 2024/4/22.
//
#include <assistant_api_v2.pb.h>
#include <gtest/gtest.h>

#include <instinct/CoreGlobals.hpp>
#include <agent.pb.h>
#include <instinct/tools/ProtobufUtils.hpp>


namespace INSTINCT_CORE_NS {
    class ProtobufUtilsTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(ProtobufUtilsTest, ConvertJSONObjectToMessage) {
        assistant::v2::MessageObject message;
        message.set_assistant_id("assistant-123");
        message.set_id("msg-123");
        auto*msg = message.add_content();
        msg->set_type(assistant::v2::MessageObject_MessageContentType::MessageObject_MessageContentType_text);
        msg->mutable_text()->set_value("hahha");

        auto *fields = message.mutable_metadata()->mutable_fields();
        fields->emplace("nullable_key", Value {});
        Value string_value;
        string_value.set_string_value("abc");
        fields->emplace("string_key", string_value);

        // this will cause trouble due to trailing zero issues
        // Value number_value;
        // number_value.set_number_value(42);
        // fields->emplace("number_key", number_value);

        Value bool_value;
        bool_value.set_bool_value(false);
        fields->emplace("bool_key", bool_value);

        Value null_value;
        null_value.set_null_value(NULL_VALUE);
        fields->emplace("null_value", null_value);

        ListValue list_value;
        list_value.add_values()->CopyFrom(string_value);
        list_value.add_values()->CopyFrom(bool_value);
        list_value.add_values()->CopyFrom(null_value);

        Value struct_value;
        struct_value.mutable_struct_value()->mutable_fields()->emplace("string_in_struct", string_value);

        fields->emplace("inner", struct_value);

        const auto json_string = ProtobufUtils::Serialize(message);
        // use `ordered_json` to preserve field order
        nlohmann::ordered_json output;
        ProtobufUtils::ConvertMessageToJsonObject(message, output);
        LOG_INFO("converted json object: {}", output.dump());
        ASSERT_EQ(output.dump(), json_string);
    }

    TEST_F(ProtobufUtilsTest, Reflection) {
        const auto descriptor = assistant::v2::AssistantObject::GetDescriptor();
        const auto reflection = assistant::v2::AssistantObject::GetReflection();

        const auto name_field = descriptor->FindFieldByName("name");
        auto* obj = new assistant::v2::AssistantObject;
        reflection->SetString(obj, name_field, "test");
        std::cout << obj->ShortDebugString() << std::endl;
        delete obj;

        assistant::v2::AssistantObject obj2;
        reflection->SetString(&obj2, name_field, "test2");
        std::cout << obj2.ShortDebugString() << std::endl;
    }

    TEST_F(ProtobufUtilsTest, ConvertMessageWithAny) {
        assistant::v2::RunStepObject_RunStepDetails details;
        llm::LLMCompilerTaskGraph graph;
        graph.set_question("hello");
        details.mutable_custom()->PackFrom(graph);
        nlohmann::ordered_json obj;
        ProtobufUtils::ConvertMessageToJsonObject(details, obj);
        std::cout << obj.dump() << std::endl;
    }


}
