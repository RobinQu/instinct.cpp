//
// Created by RobinQu on 2024/4/9.
//
#include <gtest/gtest.h>


#include "toolkit/builtin/SerpAPI.hpp"

namespace INSTINCT_AGENT_NS {
    using namespace INSTINCT_LLM_NS;

    class TestSerpAPI: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            const auto api_key = std::getenv("SERP_API_KEY");
            ASSERT_TRUE(api_key && !StringUtils::IsBlankString(api_key));
            serp_api = CreateSerpAPI({.apikey =  api_key});
        }

        std::string Query(const std::string& text) {
            FunctionToolInvocation invocation;
            SearchToolRequest search_tool_request;
            search_tool_request.set_query(text);
            invocation.set_input(ProtobufUtils::Serialize(search_tool_request));
            const auto result = serp_api->Invoke(invocation);
            assert_true(!result.has_error());
            return result.return_value();
        }

        FunctionToolPtr serp_api;
    };

    TEST_F(TestSerpAPI, GetSchema) {
        const auto schema = serp_api->GetSchema();
        LOG_INFO(">> {}", schema.ShortDebugString());
        ASSERT_EQ(schema.arguments_size(), 3);
        ASSERT_EQ(schema.arguments(0).name(), "query");
        ASSERT_EQ(schema.arguments(0).type(), "string");
        ASSERT_EQ(schema.arguments(1).name(), "result_limit");
        ASSERT_EQ(schema.arguments(1).type(), "integer");
        ASSERT_EQ(schema.arguments(2).name(), "result_offset");
        ASSERT_EQ(schema.arguments(2).type(), "integer");
    }

    TEST_F(TestSerpAPI, SimpleQuery) {
        const auto r1 = Query("how many oceans in the world?");
        LOG_INFO(">> {}", r1);

        const auto r2 = Query("what's PI?");
        LOG_INFO(">> {}", r2);

    }


}