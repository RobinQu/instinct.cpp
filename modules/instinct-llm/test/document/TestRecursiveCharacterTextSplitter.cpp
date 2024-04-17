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
        U32StringUtils::CopiesOf(5, "abcdef"),
        "",true);
        ASSERT_EQ(results.size(), 30);
        ASSERT_EQ(results[29].char32At(0), UChar32{'f'});

        results = details::split_text_with_seperator(
        "a|b|c|d|e|f",
        R"(\|)",true);
        ASSERT_EQ(results.size(), 6);
        ASSERT_EQ(results[0].char32At(0), UChar32{'a'});
        ASSERT_EQ(results[1], "|b");
        TensorUtils::PrintEmbedding(results);


        const UnicodeString original_string = "Create an Endpoint\n\nAfter your first login, you will be directed to the [Endpoint creation page](https://ui.endpoints.huggingface.co/new). As an example, this guide will go through the steps to deploy [distilbert-base-uncased-finetuned-sst-2-english](https://huggingface.co/distilbert-base-uncased-finetuned-sst-2-english) for text classification. \n\n## 1. Enter the Hugging Face Repository ID and your desired endpoint name:\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_repository.png\" alt=\"select repository\" />";
        results = details::split_text_with_seperator(original_string, "\n\n", true);
        details::print_splits(R"(Split with \n\n)", results);
        UnicodeString joined_text;
        for (const auto & result : results) {
            joined_text += result;
        }
        ASSERT_EQ(joined_text, original_string);
    }

    // Bug: https://www.diffchecker.com/0uNKwUwb/
    TEST_F(RecursiveCharacterTextSplitterTest, split_text_with_seperator_bug_1) {

    }

    TEST_F(RecursiveCharacterTextSplitterTest, SimpleSplit) {
        auto* text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 15});
        auto result = text_splitter->SplitText(U32StringUtils::CopiesOf(5, "abcdef"));
        details::print_splits("splits: ", result);
        ASSERT_TRUE(check_equality(result, std::vector {"abcdefabcdefabc", "defabcdefabcdef"}));
        delete text_splitter;

        text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 5});
        result = text_splitter->SplitText(corpus::text4);
        details::print_splits("splits: ", result);
        ASSERT_EQ(result.size(), 3);
        ASSERT_EQ(result[2].char32At(0), UChar32{U'👋'});
        delete text_splitter;
    }

    TEST_F(RecursiveCharacterTextSplitterTest, SplitWithNonEnglishText) {
        auto* text_splitter = new RecursiveCharacterTextSplitter({.chunk_size = 20});
        const auto result = text_splitter->SplitText(corpus::text6);
        details::print_splits("splits: ", result);
    }

    TEST_F(RecursiveCharacterTextSplitterTest, SplitWithTokenizer) {
        const std::filesystem::path assets_dir =
                std::filesystem::current_path() / "_assets";
        auto tokenizer =
                TiktokenTokenizer::MakeGPT4Tokenizer(assets_dir / "bpe_ranks" / "cl100k_base.tiktoken");

        auto text_splitter = CreateRecursiveCharacterTextSplitter(tokenizer, { .chunk_size = 20});
        const auto splits = text_splitter->SplitText(corpus::text3);
        details::print_splits("splits: ", splits);
    }
}
