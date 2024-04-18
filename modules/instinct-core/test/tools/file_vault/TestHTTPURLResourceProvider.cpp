//
// Created by RobinQu on 2024/4/18.
//
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "CoreGlobals.hpp"
#include "tools/file_vault/HttpURLResourceProvider.hpp"

namespace INSTINCT_CORE_NS {

    TEST(TestHttpURLResourceProvider, WithoutChecksum) {
        SetupLogging();

        auto r1_path = std::filesystem::temp_directory_path() / "r1.json";
        LOG_INFO("r1_path={}", r1_path.string());
        std::ofstream r1_of(r1_path, std::ios::out | std::ios::trunc);
        HttpURLResourceProvider r1("t1", "GET https://httpbin.org/get?a=1");
        r1.Persist(r1_of).wait();
        r1_of.close();

        std::ifstream r1_if(r1_path);
        auto data = nlohmann::json::parse(r1_if);
        ASSERT_EQ(data["args"]["a"], "1");
    }

}
