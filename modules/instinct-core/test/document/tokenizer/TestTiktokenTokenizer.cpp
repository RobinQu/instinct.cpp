//
// Created by RobinQu on 2024/3/2.
//
#include <gtest/gtest.h>
#include "document/tokenizer/TiktokenTokenizer.hpp"

namespace INSTINCT_CORE_NS {
    using namespace  U_ICU_NAMESPACE;

    static UnicodeString text1 = R"(At age nine, Swift became interested in musical theater and performed in four Berks Youth Theatre Academy productions.[19] She also traveled regularly to New York City for vocal and acting lessons.[20] Swift later shifted her focus toward country music, inspired by Shania Twain's songs, which made her "want to just run around the block four times and daydream about everything".[21] She spent weekends performing at local festivals and events.)";

    TEST(TiktokenTokenizer, TestEncodeDecodeGPT4) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "cl100k_base" / "cl100k_base.toktoken");
        // const auto ids1 = tokenizer->Encode(text1);
        // instinct::core::TensorUtils::PrintEmbedding("tokens of ids1: ", ids1);
        // ASSERT_EQ(tokenizer->Decode(ids1), text1);
        delete tokenizer;
    }

}