//
// Created by RobinQu on 3/19/24.
//
#include <gtest/gtest.h>

#include <instinct/tools/http/HttpUtils.hpp>
#include <instinct/CoreGlobals.hpp>


namespace INSTINCT_CORE_NS {

    TEST(TestHTTPUtils, CreateRequest) {
        auto req = HttpUtils::CreateRequest("POST /hello/world");
        ASSERT_EQ(req.method, kPOST);
        ASSERT_EQ(req.target, "/hello/world");
        ASSERT_EQ(req.endpoint.protocol, kUnspecifiedProtocol);
        ASSERT_EQ(req.endpoint.host, "");
        ASSERT_EQ(req.endpoint.port, 0);


        req = HttpUtils::CreateRequest("GET https://www.fancy-site.com:9999/url-may-not-exist/wow");

        ASSERT_EQ(req.method, kGET);
        ASSERT_EQ(req.target, "/url-may-not-exist/wow");
        ASSERT_EQ(req.endpoint.protocol, kHTTPS);
        ASSERT_EQ(req.endpoint.host, "www.fancy-site.com");
        ASSERT_EQ(req.endpoint.port, 9999);

    }

};

