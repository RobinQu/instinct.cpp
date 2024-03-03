//
// Created by RobinQu on 2024/3/3.
//


#include <gtest/gtest.h>

#include "document/tokenizer/Tokenizer.hpp"


namespace INSTINCT_CORE_NS {

    TEST(TestTokenizer, merge_u32_ids) {
        std::vector<int32_t> ids1 = {32, 97};
        details::merge_u32_ids(ids1, {32,97}, 256);
        TensorUtils::PrintEmbedding("merged ids: ", ids1);
        ASSERT_EQ(ids1.size(), 1);
        ASSERT_EQ(ids1[0], 256);

        std::vector<int32_t> ids2 = {98, 32, 97, 65};
        details::merge_u32_ids(ids2, {32,97}, 256);
        TensorUtils::PrintEmbedding("merged ids: ", ids2);
        ASSERT_EQ(ids2.size(), 3);
        ASSERT_EQ(ids2[0], 98);
        ASSERT_EQ(ids2[1], 256);
        ASSERT_EQ(ids2[2], 65);
    }

}