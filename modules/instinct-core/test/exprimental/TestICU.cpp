//
// Created by RobinQu on 2024/2/29.
//
#include <unicode/brkiter.h>
#include <gtest/gtest.h>
#include <unicode/regex.h>
#include <unicode/ustream.h>

#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    using namespace U_ICU_NAMESPACE;
    TEST(TestICU, SplitWithGruoup) {
        UnicodeString text = "Note that, in this example, words is a local, or stack array of actual UnicodeString objects. No heap allocation is involved in initializing this array of empty strings (C++ is not Java!). Local UnicodeString arrays like this are a very good fit for use with split(); after extracting the fields, any values that need to be kept in some more permanent way can be copied to their ultimate destination.";
        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher m("(\\s+)", 0, status);
        const int maxWords = 10;
        UnicodeString words[maxWords];
        int numWords = m.split(text, words, maxWords, status);
        for(int i =0; i<numWords; i++) {
            std::cout << words[i] << std::endl;
        }
    }

    TEST(TestICU, W32Char) {
        std::string s1 = "今天😊天气 Good weather today";
        UnicodeString s2 = UnicodeString::fromUTF8(s1);
        std::string s3;
        s2.toUTF8String(s3);
        auto print = [](const std::string& s) {
            for(const auto& c: s) {
                std::cout << int32_t(c) << ",";
            }
            std::cout << std::endl;
        };
        print(s1);
        print(s3);
    }
}