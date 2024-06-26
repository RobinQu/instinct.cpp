//
// Created by RobinQu on 2024/3/5.
//

#include <gtest/gtest.h>
#include <instinct/document/CharacterTextSplitter.hpp>
#include <instinct/document/LanguageSplitters.hpp>
#include <instinct/tools/StringUtils.hpp>

#include "Corpus.hpp"


namespace INSTINCT_LLM_NS {
    TEST(TestCharacterTextSplitter, SplitEnText) {
        auto* text_splitter = new CharacterTextSplitter({.chunk_size = 100});
        auto result = text_splitter->SplitText(corpus::text5);
        details::print_splits("splits: ", result);
        ASSERT_EQ(result.size(), 4);
        delete text_splitter;

        text_splitter = new CharacterTextSplitter({.chunk_size = 7, .chunk_overlap = 3, .separator=" "});
        result = text_splitter->SplitText("abc def jkl mna");
        details::print_splits("splits: ", result);
        ASSERT_EQ(result.size(), 3);
        ASSERT_TRUE(result[2] == "jkl mna");
        delete text_splitter;
    }

    TEST(TestCharacterTextSplitter, SplitCnText) {
        auto* text_splitter = new CharacterTextSplitter({.chunk_size = 5, .chunk_overlap = 2, .separator =   " "});
        auto result = text_splitter->SplitText("朱雀 玄武 青龙 白虎");
        details::print_splits("cn splits: ", result);
        ASSERT_EQ(result.size(), 3);
        ASSERT_EQ(result[2], "青龙 白虎");
        delete text_splitter;
    }


}
