//
// Created by RobinQu on 2024/4/20.
//
#include <gtest/gtest.h>
#include <assistant_api_v2.pb.h>
#include <google/protobuf/util/json_util.h>

namespace INSTINCT_CORE_NS {
    TEST(ProtobufTest, JSONConversion) {
        instinct::assistant::v2::AssistantObject ao;
        const auto fields = ao.mutable_metadata()->mutable_fields();
        google::protobuf::Value v;
        v.set_bool_value(false);
        fields->emplace("k", v);
        std::string output;
        google::protobuf::util::MessageToJsonString(ao, &output);
        std::cout << output << std::endl;
        ASSERT_EQ(R"""({"metadata":{"k":false}})""", output);
    }
}