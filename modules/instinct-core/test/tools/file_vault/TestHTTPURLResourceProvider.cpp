//
// Created by RobinQu on 2024/4/18.
//
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include <instinct/CoreGlobals.hpp>
#include <instinct/tools/CodecUtils.hpp>
#include <instinct/tools/HashUtils.hpp>
#include <instinct/tools/file_vault/HttpURLResourceProvider.hpp>

namespace INSTINCT_CORE_NS {

    TEST(TestHttpURLResourceProvider, SimpleTest) {
        SetupLogging();
        const auto id = StringUtils::GenerateUUIDString();
        auto r1_path = std::filesystem::temp_directory_path() / "r1.json";
        LOG_INFO("r1_path={}", r1_path.string());
        std::ofstream r1_of(r1_path, std::ios::out | std::ios::trunc);
        HttpURLResourceProvider r1("t1", "GET https://httpbin.org/get?id="  + id);
        r1.Persist(r1_of).get();
        r1_of.close();

        std::ifstream r1_if(r1_path);
        auto data = nlohmann::json::parse(r1_if);
        ASSERT_EQ(data["args"]["id"], id);

        ASSERT_EQ(r1.GetResourceName(), "t1");
        ASSERT_EQ(r1.GetChecksum().algorithm, kNoChecksum);
        ASSERT_EQ(r1.GetChecksum().expected_value, "");
    }

    TEST(TestHttpURLResourceProvider, RejectResponseInvalidStatusCode) {
        HttpURLResourceProvider r1("t2", "GET https://httpbin.org/status/500");
        ASSERT_THROW({
            std::stringstream ss;
            r1.Persist(ss).get();
        }, InstinctException);
    }



}
