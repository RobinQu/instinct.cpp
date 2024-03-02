//
// Created by RobinQu on 2024/2/29.
//
#include <unicode/brkiter.h>
#include <gtest/gtest.h>
#include <unicode/regex.h>
#include <unicode/ustream.h>

#include "CoreGlobals.hpp"
#include "document/tokenizer/Tokenizer.hpp"
#include "prompt/FewShotPromptTemplate.hpp"
#include "tools/TensorUtils.hpp"

namespace INSTINCT_CORE_NS {
    using namespace U_ICU_NAMESPACE;

    static UnicodeString text1 = "Note that, in this example, words is a local, or stack array of actual UnicodeString objects. No heap allocation is involved in initializing this array of empty strings (C++ is not Java!). Local UnicodeString arrays like this are a very good fit for use with split(); after extracting the fields, any values that need to be kept in some more permanent way can be copied to their ultimate destination.";
    static UnicodeString text2 = "hello 你好 😊";

    TEST(TestICU, SplitWithGruoup) {

        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher m("(\\s+)", 0, status);
        const int maxWords = 10;
        UnicodeString words[maxWords];
        int numWords = m.split(text1, words, maxWords, status);
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


    TEST(TestICU, TestBuildRanks) {
        // std::vector<UChar32> rank_to_int;
        for(int i=0;i<256;i++) {
            UnicodeString tmp(i);
            if (u_isprint(i) && tmp != ' ') {
                std::cout << "printable: " << tmp << std::endl;
            }
        }
    }


    TEST(TestICU, split_text_with_regex) {

        // split with category
        auto reg_pattern = "\\s+";
        std::vector<UnicodeString> result;
        details::split_text_with_regex(text2, reg_pattern, result);
        for (const auto& str: result) {
            std::cout << "length=" << str.length() << ", content=" << str << std::endl;
        }
        ASSERT_EQ(result.size(), 3);

        // split with simpe char
        result.clear();
        details::split_text_with_regex("aaaabbccbc", "b", result);
        TensorUtils::PrintEmbedding("splits: ", result);
        ASSERT_EQ(result.size(), 4);

        // split with non-existence char

        result.clear();
        details::split_text_with_regex("aaaabbccbc", "f", result);
        TensorUtils::PrintEmbedding("splits: ", result);



    }

    TEST(TestICU, find_all_with_regex) {
        auto reg_pattern = UnicodeString::fromUTF8(R"""('(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?+\p{L}+|\p{N}{1,3}| ?[^\s\p{L}\p{N}]++[\r\n]*|\s*[\r\n]|\s+(?!\S)|\s+)""");
        std::vector<UnicodeString> result;
        details::find_all_with_regex(text2, reg_pattern, result);
        TensorUtils::PrintEmbedding("text2 output: ", result);

        result.clear();
        details::find_all_with_regex(text1, reg_pattern, result);
        TensorUtils::PrintEmbedding("text1 output: ", result);
    }

    TEST(TestICU, escape_for_regular_expression) {
        UnicodeString t = R"(\.^$-+()[]{}|?*)";
        auto output = details::escape_for_regular_expression(t);
        std::cout << output << std::endl;
    }





}
