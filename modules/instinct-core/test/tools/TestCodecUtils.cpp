//
// Created by RobinQu on 2024/3/18.
//
#include <gtest/gtest.h>

#include <instinct/CoreGlobals.hpp>
#include <instinct/tools/CodecUtils.hpp>

namespace INSTINCT_CORE_NS {

    static std::unordered_map<std::string, std::string> BASE64_TEST_PARIS {
            { "",		""         },
            { "f",		"Zg=="     },
            { "fo",		"Zm8="     },
            { "foo",	"Zm9v"     },
            { "foob",	"Zm9vYg==" },
            { "fooba",	"Zm9vYmE=" },
            { "foobar",	"Zm9vYmFy" }
    };


    TEST(TestCodecUtils, Base64) {
        for (const auto& [decoded,encoded]: BASE64_TEST_PARIS) {
            ASSERT_EQ(CodecUtils::EncodeBase64(decoded), encoded);
            ASSERT_EQ(CodecUtils::DecodeBase64(encoded), decoded);
        }
    }




}


