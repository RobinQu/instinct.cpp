//
// Created by RobinQu on 2024/6/17.
//

#include <gtest/gtest.h>
#include "embedding_model/LocalEmbeddingModel.hpp"
#include "tools/TensorUtils.hpp"


namespace INSTINCT_LLM_NS {


    class LocalEmbeddingModelTest: public testing::Test {
    protected:
        void SetUp() override {
            SetupLogging();
            PreloadEmbeddingModelFiles();
        }
    };


    TEST_F(LocalEmbeddingModelTest, BGE_M3_EMBEDDING) {
        const auto embedding_model = CreateLocalRankingModel(ModelType::BGE_M3_EMBEDDING);
        ASSERT_EQ(embedding_model->GetDimension(), 1024);
        const auto v1 = embedding_model->EmbedQuery("hello world");
        TensorUtils::PrintEmbedding(v1);
    }




}
