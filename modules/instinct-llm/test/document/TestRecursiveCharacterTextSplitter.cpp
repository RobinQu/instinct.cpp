//
// Created by RobinQu on 2024/3/4.
//
#include <gtest/gtest.h>


#include "CoreGlobals.hpp"
#include "document/RecursiveCharacterTextSplitter.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"
#include "tokenizer/Tokenizer.hpp"

#include "Corpus.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_LLM_NS {


    class RecursiveCharacterTextSplitterTest : public ::testing::Test {

    protected:
        void SetUp() override {
            SetupLogging();
        }
    };

    TEST_F(RecursiveCharacterTextSplitterTest, split_text_with_seperator) {
        auto results = details::split_text_with_seperator(
        u32_utils::copies_of(5, "abcdef"),
        "",true);
        ASSERT_EQ(results.size(), 30);
        ASSERT_EQ(results[29].char32At(0), UChar32{'f'});
    }

    TEST_F(RecursiveCharacterTextSplitterTest, SimpleSplit) {
        auto* text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 15});
        auto result = text_splitter->SplitText(u32_utils::copies_of(5, "abcdef"));
        u32_utils::print_splits("splits: ", result);
        ASSERT_TRUE(check_equality(result, std::vector {"abcdefabcdefabc", "defabcdefabcdef"}));

        text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 5});
        result = text_splitter->SplitText(corpus::text4);
        u32_utils::print_splits("splits: ", result);
        ASSERT_EQ(result.size(), 3);
        ASSERT_EQ(result[2].char32At(0), UChar32{U'👋'});
    }

    TEST_F(RecursiveCharacterTextSplitterTest, SplitWithNonEnglishText) {
        auto* text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 20});
        const auto result = text_splitter->SplitText(corpus::text6);
        u32_utils::print_splits("splits: ", result);
    }

    TEST_F(RecursiveCharacterTextSplitterTest, SplitWithTokenizer) {
        const std::filesystem::path assets_dir =
                std::filesystem::current_path() / "_assets";
        auto tokenizer =
                TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "bpe_ranks" / "cl100k_base.tiktoken");

        auto text_splitter = CreateRecursiveCharacterTextSplitter(tokenizer, { .chunk_size = 20});
        const auto splits = text_splitter->SplitText(corpus::text3);
        u32_utils::print_splits("splits: ", splits);

    }
}
