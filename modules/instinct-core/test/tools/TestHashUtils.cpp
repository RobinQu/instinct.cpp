//
// Created by RobinQu on 2024/4/18.
//
#include <gtest/gtest.h>

#include <instinct/CoreGlobals.hpp>
#include <instinct/tools/Assertions.hpp>
#include <instinct/tools/HashUtils.hpp>

namespace INSTINCT_CORE_NS {

    TEST(TestHashUtils, HashString) {
        ASSERT_EQ(HashUtils::HashForString<MD5>("helloworld"), "fc5e038d38a57032085441e7fe7010b0");
        ASSERT_EQ(HashUtils::HashForString<SHA1>("helloworld"), "6adfb183a4a2c94a2f92dab5ade762a47889a5a1");
        ASSERT_EQ(HashUtils::HashForString<SHA256>("helloworld"), "936a185caaa266bb9cbe981e9e05cb78cd732b0b3280eb944412bb6f8f8f07af");
    }

    TEST(TestHashUtils, HashStream) {
        std::stringstream ss;
        ss << "helloworld";
        ASSERT_EQ(HashUtils::HashForStream<MD5>(ss), "fc5e038d38a57032085441e7fe7010b0");
        ss.clear();
        ss << "helloworld";
        ASSERT_EQ(HashUtils::HashForStream<SHA1>(ss), "6adfb183a4a2c94a2f92dab5ade762a47889a5a1");
        ss.clear();
        ss << "helloworld";
        ASSERT_EQ(HashUtils::HashForStream<SHA256>(ss), "936a185caaa266bb9cbe981e9e05cb78cd732b0b3280eb944412bb6f8f8f07af");
    }

}
