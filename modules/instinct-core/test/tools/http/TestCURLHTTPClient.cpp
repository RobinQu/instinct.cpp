//
// Created by RobinQu on 3/19/24.
//

#include <gtest/gtest.h>
#include "tools/http/CURLHttpClient.hpp"


namespace INSTINCT_CORE_NS {

    class CURLHttpClientTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(CURLHttpClientTest, SimpleRequest) {
        CURLHttpClient client;
        auto req1 = HttpUtils::CreateRequest("GET https://httpbin.org/get?foo=bar");
        auto resp1 = client.Execute(req1);
        LOG_INFO("req1: status_code={}, body={}", resp1.status_code, resp1.body);
        ASSERT_TRUE(!resp1.headers.empty());

        auto req2 = HttpUtils::CreateRequest("POST https://httpbin.org/post");
        req2.body = R"({"hello": "world"})";
        auto resp2 = client.Execute(req2);
        LOG_INFO("req2: status_code={}, body={}", resp2.status_code, resp2.body);
        ASSERT_TRUE(!resp2.headers.empty());


        auto req3 = HttpUtils::CreateRequest("PUT https://httpbin.org/status/201");
        req2.body = R"({"hello": "world"})";
        auto resp3 = client.Execute(req3);
        LOG_INFO("req3: status_code={}, body={}", resp3.status_code, resp3.body);
        ASSERT_TRUE(!resp3.headers.empty());
        ASSERT_EQ(resp3.status_code, 201);


        auto req4 = HttpUtils::CreateRequest("DELETE https://httpbin.org/status/204");
        auto resp4 = client.Execute(req4);
        LOG_INFO("resp4: status_code={}, body={}", resp4.status_code, resp4.body);
        ASSERT_TRUE(!resp4.headers.empty());
        ASSERT_EQ(resp4.status_code, 204);

        auto req5 = HttpUtils::CreateRequest("PUT https://httpbin.org/put");
        req5.headers[HTTP_HEADER_CONTENT_TYPE_NAME] = HTTP_CONTENT_TYPES.at(kJSON);
        req5.body = R"({"hello": "world"})";
        auto resp5 = client.Execute(req5);
        LOG_INFO("resp5: status_code={}, body={}", resp5.status_code, resp5.body);
        ASSERT_TRUE(!resp5.headers.empty());
        ASSERT_EQ(HttpUtils::GetHeaderValue("Content-Type", "", resp5.headers), "application/json");
    }


    /**
     * This test relies on a locally running OpenAI-compatible API service. This extra dependency will be addressed in the future.
     */
    TEST_F(CURLHttpClientTest, ChunkedResponse) {
        CURLHttpClient client;
        const auto req1 = HttpUtils::CreateRequest("GET https://httpbin.org/stream/3");
        client.StreamChunk(req1, {.line_breaker = "\n"})
            .subscribe([](const auto& chunk) {
                LOG_INFO("chunk: {}", chunk);
            });
    }

    TEST_F(CURLHttpClientTest, BatchRequest) {
        using namespace std::chrono_literals;
        ThreadPool pool {2};
        CURLHttpClient client;
        constexpr int n = 5;
        std::vector<HttpRequest> calls;
        calls.reserve(n);
        for(int i=0;i<n;i++) {
            calls.push_back({
                .endpoint = {.protocol = kHTTP, .host="httpbin.org", .port=80},
                .method = kPOST,
                .target = "/post",

            });
        }
        auto futures = client.ExecuteBatch(calls, pool);

        for(auto& f: futures) {
            auto resp = f.get();
            ASSERT_EQ(resp.status_code, 200);
            ASSERT_TRUE(StringUtils::IsNotBlankString(resp.body));
        }

    }

}
