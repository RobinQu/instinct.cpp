//
// Created by RobinQu on 2024/3/2.
//
#include <gtest/gtest.h>
#include "document/tokenizer/TiktokenTokenizer.hpp"
#include "tools/Assertions.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_CORE_NS {
    using namespace  U_ICU_NAMESPACE;

    static UnicodeString text1 = R"(At age nine, Swift became interested in musical theater and performed in four Berks Youth Theatre Academy productions.[19] She also traveled regularly to New York City for vocal and acting lessons.[20] Swift later shifted her focus toward country music, inspired by Shania Twain's songs, which made her "want to just run around the block four times and daydream about everything".[21] She spent weekends performing at local festivals and events.)";


    TEST(TiktokenTokenizer, recover_byte_pair_bpe_ranks) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        auto tiktoken_bpe_file_path = assets_dir / "cl100k_base" / "cl100k_base.tiktoken";

        auto t1 = ChronoUtils::GetCurrentTimeMillis();
        TiktokenBPEFileReader reader(tiktoken_bpe_file_path);
        auto bpe_token = reader.Fetch();
        std::cout << "bpe loaded in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;

        //
        // t1 = ChronoUtils::GetCurrentTimeMillis();
        // for (int i=0;const auto& [token, rank]: bpe_token) {
        //     // empty loop
        //     if(i++ % 1000 == 0) {
        //         std::cout << "1000 items" << std::endl;
        //     }
        // }
        // std::cout << "loop bpe_ranks in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;

        t1 = ChronoUtils::GetCurrentTimeMillis();
        auto bpe = details::recover_byte_pair_bpe_ranks(bpe_token);
        std::cout << "recover_byte_pair_bpe_ranks in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;

        ASSERT_EQ(bpe.size(), 100000);

    }

    TEST(TiktokenTokenizer, TestEncodeDecodeGPT4) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "cl100k_base" / "cl100k_base.tiktoken");
        const auto ids1 = tokenizer->Encode(text1);
        TensorUtils::PrintEmbedding("tokens of ids1: ", ids1);
        ASSERT_EQ(tokenizer->Decode(ids1), text1);

        const auto ids2 = tokenizer->Encode("hello world 👋");
        TensorUtils::PrintEmbedding("tokens of ids2: ", ids2);
        ASSERT_TRUE(check_equality(ids2, {15339, 1917, 62904, 233}));

        delete tokenizer;
    }

}
