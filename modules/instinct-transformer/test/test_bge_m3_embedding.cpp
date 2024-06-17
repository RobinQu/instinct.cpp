//
// Created by RobinQu on 2024/6/17.
//

#include <filesystem>
#include <gtest/gtest.h>

#include "models//bge_embedding.hpp"
#include "model_factory.hpp"

namespace INSTINCT_TRANSFORMER_NS {

    class BGEM3EmbeddingTest: public testing::Test {
    protected:
        const std::filesystem::path bge_m3_ranker_bin = std::filesystem::current_path() / "_assets/model_bins/bge-m3e.bin";
        ModelFactory model_factory;
        TokenizerPtr tokenizer_;
        ModelPtr model_;


        void SetUp() override {
            std::tie(this->model_, this->tokenizer_) = model_factory.load(this->bge_m3_ranker_bin.string());
        }
    };

    TEST_F(BGEM3EmbeddingTest, Embedding) {
        constexpr models::GenerationConfig config {.num_threads = 1};
        std::vector<float> embedding;
        const auto tokens = tokenizer_->encode("hello");
        for(const auto& n: tokens) {
            std::cout << n << " ";
        }
        std::cout << std::endl;

        model_->text_embedding(config, tokens, embedding);
        for(const auto& n: embedding) {
            // std::cout << std::setw(14) << std::fixed << std::setprecision(8) << n << " ";
            std::cout << n << " ";
        }
        std::cout << std::endl;

        ASSERT_EQ(embedding.front(), -0.0311382655f);
        ASSERT_EQ(embedding.back(), 0.060088817f);
    }
}
