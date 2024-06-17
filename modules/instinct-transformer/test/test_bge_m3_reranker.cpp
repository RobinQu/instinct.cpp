//
// Created by RobinQu on 2024/5/26.
//


#include <gtest/gtest.h>
#include <filesystem>

#include "model_factory.hpp"

namespace INSTINCT_TRANSFORMER_NS {
    class BGEM3RankerTest: public testing::Test {
    protected:
        const std::filesystem::path bge_m3_ranker_bin = std::filesystem::current_path() / "_assets/model_bins/bge-reranker-v2-m3.bin";

        ModelFactory model_factory;
        TokenizerPtr tokenizer_;
        ModelPtr model_;

        void SetUp() override {
            load_bge_m3_ranker();
        }

        void load_bge_m3_ranker() {
            std::tie(this->model_, this->tokenizer_) = model_factory.load(this->bge_m3_ranker_bin.string());
        }

        [[nodiscard]] float get_rank_score(const std::string &q, const std::string& a) const {
            constexpr GenerationConfig config {.num_threads = 1};
            std::vector<int> ids;
            this->tokenizer_->encode_qa(q, a, ids);
            return this->model_->qa_rank(config, ids);
        }
    };

    TEST_F(BGEM3RankerTest, test_encoder) {
        ASSERT_EQ(
            tokenizer_->encode("hello"),
            std::vector({0, 33600, 31, 2})
        );
    }

    TEST_F(BGEM3RankerTest, test_ranker) {
        ASSERT_GT(get_rank_score("hello", "welcome"), 0.7f);
        ASSERT_LT(get_rank_score("hello", "farewell"), 0.1);
    }
}

