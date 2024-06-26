//
// Created by RobinQu on 2024/4/23.
//
#include <gtest/gtest.h>
#include <instinct/tools/StringUtils.hpp>

namespace INSTINCT_CORE_NS {
    TEST(StringUtilsTest, EscapeSQLText) {
        ASSERT_EQ(StringUtils::EscapeSQLText(R"(What's 1 plus one)"), "What''s 1 plus one");
    }
}