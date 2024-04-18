//
// Created by RobinQu on 2024/4/18.
//
#include <gtest/gtest.h>
#include "CoreGlobals.hpp"
#include "tools/file_vault/HttpURLResourceProvider.hpp"

namespace INSTINCT_CORE_NS {

    TEST(TestHttpURLResourceProvider, WithoutChecksum) {
        HttpURLResourceProvider r1("t1", "GET https://httpbin.org/get");
    }

}
