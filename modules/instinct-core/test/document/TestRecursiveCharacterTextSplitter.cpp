//
// Created by RobinQu on 2024/3/4.
//
#include <gtest/gtest.h>


#include "CoreGlobals.hpp"
#include "document/RecursiveCharacterTextSplitter.hpp"
#include "document/tokenizer/TiktokenTokenizer.hpp"
#include "document/tokenizer/Tokenizer.hpp"

#include "Corpus.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_CORE_NS {
    TEST(RecursiveCharacterTextSplitter, TestSimpleSplit) {
        RecursiveCharacterTextSplitter text_splitter({.chunk_size = 5, .chunk_overlap = 0});
        auto result = text_splitter.SplitText(StringUtils::CopiesOf(5, "abcdef"));
        ASSERT_TRUE(check_equality(result, std::vector {"abcdefabcdefabc", "defabcdefabcdef"}));
    }

    TEST(RecursiveCharacterTextSplitter, TestSplitWithTokenizer) {
        const std::filesystem::path assets_dir =
                std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        auto* tokenizer =
                TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "cl100k_base" / "cl100k_base.tiktoken");

        auto* text_splitter = RecursiveCharacterTextSplitter::FromTiktokenTokenizer(tokenizer);
        auto splits = text_splitter->SplitText(corpus::text2);
        TensorUtils::PrintEmbedding("splits=", splits);

        delete tokenizer;
        delete text_splitter;
    }
}
