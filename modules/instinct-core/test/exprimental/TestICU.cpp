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
    static UnicodeString text3 = R"(<|endoftext|>The llama (/ˈlɑːmə/; Spanish pronunciation: [ˈʎama] or [ˈʝama]) (Lama glama) is a domesticated South American camelid, widely used as a meat and pack animal by Andean cultures since the pre-Columbian era.
Llamas are social animals and live with others as a herd. Their wool is soft and contains only a small amount of lanolin.[2] Llamas can learn simple tasks after a few repetitions. When using a pack, they can carry about 25 to 30% of their body weight for 8 to 13 km (5–8 miles).[3] The name llama (in the past also spelled "lama" or "glama") was adopted by European settlers from native Peruvians.[4]
The ancestors of llamas are thought to have originated from the Great Plains of North America about 40 million years ago, and subsequently migrated to South America about three million years ago during the Great American Interchange. By the end of the last ice age (10,000–12,000 years ago), camelids were extinct in North America.[3] As of 2007, there were over seven million llamas and alpacas in South America and over 158,000 llamas and 100,000 alpacas, descended from progenitors imported late in the 20th century, in the United States and Canada.[5]
<|fim_prefix|>In Aymara mythology, llamas are important beings. The Heavenly Llama is said to drink water from the ocean and urinates as it rains.[6] According to Aymara eschatology,<|fim_suffix|> where they come from at the end of time.[6]<|fim_middle|> llamas will return to the water springs and ponds<|endofprompt|>)";

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

    TEST(TestICU, split_with_special_token) {
        UErrorCode status = U_ZERO_ERROR;

        RegexMatcher matcher(R"((<\|endoftext\|>|<\|fim_prefix\|>|<\|fim_middle\|>|<\|fim_suffix\|>|<\|endofprompt\|>))", 0, status);
        if(U_FAILURE(status)) {
            throw InstinctException("boom");
        }
        const int max_size = 10;
        UnicodeString parts[max_size];

        int split_size;
        UnicodeString text = text3;
        int n = 0;
        do {
            split_size = matcher.split(text, parts, max_size, status);
            if(U_FAILURE(status)) {
                throw InstinctException("boom");
            }
            std::cout << "split_size=" << split_size << std::endl;
            for (int i=0;i<split_size;i++) {
                std::cout << "parts[" << n++ << "]=" << parts[i] << std::endl;
            }
            text = parts[max_size-1];
        } while(split_size == max_size);
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

        result.clear();
        auto specials = {"<|endoftext|>", "<|fim_prefix|>", "<|fim_middle|>", "<|fim_suffix|>", "<|endofprompt|>"};
        auto specials_unicode = specials | std::views::transform([](const auto& str) {
            return details::escape_for_regular_expression(str);
        });
        UnicodeString pattern_str = "(" + details::join_with_seperator("|", specials_unicode) + ")";
        std::cout << "pattern string: " << pattern_str << std::endl;
        details::split_text_with_regex(text3, pattern_str, result);
        TensorUtils::PrintEmbedding("result=", result);
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
