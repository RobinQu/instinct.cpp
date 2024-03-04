//
// Created by RobinQu on 2024/3/2.
//

#ifndef TESTREGEXPTOKENIZER_HPP
#define TESTREGEXPTOKENIZER_HPP


#include <gtest/gtest.h>

#include "document/tokenizer/RegexTokenizer.hpp"
#include "tools/TensorUtils.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_CORE_NS {
    static UnicodeString text1 = UnicodeString::fromUTF8(R"""(<|endoftext|>The llama (/ˈlɑːmə/; Spanish pronunciation: [ˈʎama] or [ˈʝama]) (Lama glama) is a domesticated South American camelid, widely used as a meat and pack animal by Andean cultures since the pre-Columbian era.
Llamas are social animals and live with others as a herd. Their wool is soft and contains only a small amount of lanolin.[2] Llamas can learn simple tasks after a few repetitions. When using a pack, they can carry about 25 to 30% of their body weight for 8 to 13 km (5–8 miles).[3] The name llama (in the past also spelled "lama" or "glama") was adopted by European settlers from native Peruvians.[4]
The ancestors of llamas are thought to have originated from the Great Plains of North America about 40 million years ago, and subsequently migrated to South America about three million years ago during the Great American Interchange. By the end of the last ice age (10,000–12,000 years ago), camelids were extinct in North America.[3] As of 2007, there were over seven million llamas and alpacas in South America and over 158,000 llamas and 100,000 alpacas, descended from progenitors imported late in the 20th century, in the United States and Canada.[5]
<|fim_prefix|>In Aymara mythology, llamas are important beings. The Heavenly Llama is said to drink water from the ocean and urinates as it rains.[6] According to Aymara eschatology,<|fim_suffix|> where they come from at the end of time.[6]<|fim_middle|> llamas will return to the water springs and ponds<|endofprompt|>)""");

    TEST(RegexTokenizer, TestEncode) {

        auto reg_pattern = UnicodeString::fromUTF8(R"""('(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?+\p{L}+|\p{N}{1,3}| ?[^\s\p{L}\p{N}]++[\r\n]*|\s*[\r\n]|\s+(?!\S)|\s+)""");
        RegexTokenizer regexp_tokenizer(reg_pattern, {
                    {"<|endoftext|>", 100257},
                    {"<|fim_prefix|>", 100258},
                    {"<|fim_middle|>", 100259},
                    {"<|fim_suffix|>", 100260},
                    {"<|endofprompt|>", 100276}
                });
        regexp_tokenizer.Train(text1, 512);
        const auto ids = regexp_tokenizer.Encode("hello", {.allow_special = kAll});
        TensorUtils::PrintEmbedding(ids);
        ASSERT_TRUE(check_equality(ids, std::vector{291,297,111}));
        auto ret1 = regexp_tokenizer.Decode(ids);
        std::cout << ret1 << std::endl;
        // Decode(Encode(text)) should be text
        ASSERT_EQ(ret1, "hello");
    }

}


#endif //TESTREGEXPTOKENIZER_HPP
