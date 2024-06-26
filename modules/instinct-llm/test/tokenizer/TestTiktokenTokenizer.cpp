//
// Created by RobinQu on 2024/3/2.
//
#include <gtest/gtest.h>
#include <instinct/tokenizer/TiktokenTokenizer.hpp>
#include <instinct/tools/Assertions.hpp>
#include <instinct/tools/ChronoUtils.hpp>

namespace INSTINCT_LLM_NS {
    using namespace  U_ICU_NAMESPACE;

    static UnicodeString text1 = R"(At age nine, Swift became interested in musical theater and performed in four Berks Youth Theatre Academy productions.[19] She also traveled regularly to New York City for vocal and acting lessons.[20] Swift later shifted her focus toward country music, inspired by Shania Twain's songs, which made her "want to just run around the block four times and daydream about everything".[21] She spent weekends performing at local festivals and events.)";

    static UnicodeString text2 = R"(<|endoftext|>The llama (/ˈlɑːmə/; Spanish pronunciation: [ˈʎama] or [ˈʝama]) (Lama glama) is a domesticated South American camelid, widely used as a meat and pack animal by Andean cultures since the pre-Columbian era.
Llamas are social animals and live with others as a herd. Their wool is soft and contains only a small amount of lanolin.[2] Llamas can learn simple tasks after a few repetitions. When using a pack, they can carry about 25 to 30% of their body weight for 8 to 13 km (5–8 miles).[3] The name llama (in the past also spelled "lama" or "glama") was adopted by European settlers from native Peruvians.[4]
The ancestors of llamas are thought to have originated from the Great Plains of North America about 40 million years ago, and subsequently migrated to South America about three million years ago during the Great American Interchange. By the end of the last ice age (10,000–12,000 years ago), camelids were extinct in North America.[3] As of 2007, there were over seven million llamas and alpacas in South America and over 158,000 llamas and 100,000 alpacas, descended from progenitors imported late in the 20th century, in the United States and Canada.[5]
<|fim_prefix|>In Aymara mythology, llamas are important beings. The Heavenly Llama is said to drink water from the ocean and urinates as it rains.[6] According to Aymara eschatology,<|fim_suffix|> where they come from at the end of time.[6]<|fim_middle|> llamas will return to the water springs and ponds<|endofprompt|>)";


    TEST(TiktokenTokenizer, recover_byte_pair_bpe_ranks) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "_assets";
        auto tiktoken_bpe_file_path = assets_dir / "bpe_ranks" / "cl100k_base.tiktoken";

        auto t1 = ChronoUtils::GetCurrentTimeMillis();
        TiktokenBPEFileReader reader(tiktoken_bpe_file_path);
        auto bpe_token = reader.Fetch();
        std::cout << "bpe loaded in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;

        t1 = ChronoUtils::GetCurrentTimeMillis();
        auto bpe = details::recover_byte_pair_bpe_ranks(bpe_token);
        std::cout << "recover_byte_pair_bpe_ranks in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;

        ASSERT_EQ(bpe.size(), 100000);

    }

    TEST(TiktokenTokenizer, TestEncodeDecodeGPT4) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "_assets";
        const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "bpe_ranks" / "cl100k_base.tiktoken");
        const auto ids1 = tokenizer->Encode(text1);
        TensorUtils::PrintEmbedding("tokens of ids1: ", ids1);
        ASSERT_EQ(tokenizer->Decode(ids1), text1);

        const auto ids2 = tokenizer->Encode("hello world 👋");
        TensorUtils::PrintEmbedding("tokens of ids2: ", ids2);
        ASSERT_TRUE(check_equality(ids2, std::vector{15339, 1917, 62904, 233}));

    }

    TEST(TiktokenTokenizer, TestEncodeWithSepcials) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "_assets";
        const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "bpe_ranks" / "cl100k_base.tiktoken");
        const auto ids3 = tokenizer->Encode(text2, {.allow_special = kAll});
        TensorUtils::PrintEmbedding("tokens of ids3: ", ids3);
        ASSERT_TRUE(check_equality(ids3, std::vector{100257, 791, 94776, 47325, 135, 230, 75, 133, 239, 135, 238, 76, 99638, 14, 26, 15506, 71722, 25, 510, 135, 230, 134, 236, 3105, 60, 477, 510, 135, 230, 134, 251, 3105, 2526, 320, 43, 3105, 2840, 3105, 8, 374, 264, 13018, 660, 4987, 3778, 50252, 307, 11, 13882, 1511, 439, 264, 13339, 323, 3854, 10065, 555, 1628, 5420, 27833, 2533, 279, 864, 7813, 1152, 13464, 11639, 627, 43, 24705, 300, 527, 3674, 10099, 323, 3974, 449, 3885, 439, 264, 59213, 13, 11205, 39640, 374, 8579, 323, 5727, 1193, 264, 2678, 3392, 315, 31791, 37737, 8032, 17, 60, 445, 24705, 300, 649, 4048, 4382, 9256, 1306, 264, 2478, 86066, 13, 3277, 1701, 264, 3854, 11, 814, 649, 6920, 922, 220, 914, 311, 220, 966, 4, 315, 872, 2547, 4785, 369, 220, 23, 311, 220, 1032, 13437, 320, 20, 4235, 23, 8931, 94638, 18, 60, 578, 836, 94776, 320, 258, 279, 3347, 1101, 68918, 330, 81101, 1, 477, 330, 6200, 3105, 909, 574, 18306, 555, 7665, 61107, 505, 10068, 3700, 12328, 5493, 8032, 19, 933, 791, 38618, 315, 9507, 29189, 527, 3463, 311, 617, 44853, 505, 279, 8681, 63911, 315, 4892, 5270, 922, 220, 1272, 3610, 1667, 4227, 11, 323, 28520, 73691, 311, 4987, 5270, 922, 2380, 3610, 1667, 4227, 2391, 279, 8681, 3778, 5783, 3455, 13, 3296, 279, 842, 315, 279, 1566, 10054, 4325, 320, 605, 11, 931, 4235, 717, 11, 931, 1667, 4227, 705, 50252, 3447, 1051, 69918, 304, 4892, 5270, 8032, 18, 60, 1666, 315, 220, 1049, 22, 11, 1070, 1051, 927, 8254, 3610, 9507, 29189, 323, 453, 46051, 300, 304, 4987, 5270, 323, 927, 220, 11286, 11, 931, 9507, 29189, 323, 220, 1041, 11, 931, 453, 46051, 300, 11, 58842, 505, 84360, 12170, 25973, 3389, 304, 279, 220, 508, 339, 9478, 11, 304, 279, 3723, 4273, 323, 7008, 8032, 20, 933, 100258, 644, 362, 1631, 5169, 59492, 11, 9507, 29189, 527, 3062, 23837, 13, 578, 88150, 445, 81101, 374, 1071, 311, 7172, 3090, 505, 279, 18435, 323, 4433, 258, 988, 439, 433, 62555, 8032, 21, 60, 10771, 311, 362, 1631, 5169, 1560, 9884, 2508, 11, 100260, 1405, 814, 2586, 505, 520, 279, 842, 315, 892, 8032, 21, 60, 100259, 9507, 29189, 690, 471, 311, 279, 3090, 42242, 323, 89455, 100276}));
    }

}
