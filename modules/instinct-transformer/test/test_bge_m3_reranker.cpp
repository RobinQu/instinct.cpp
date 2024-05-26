//
// Created by RobinQu on 2024/5/26.
//


#include <gtest/gtest.h>
#include <filesystem>

#include "model_factory.hpp"

namespace INSTINCT_TRANSFORMER_NS {
    static float get_rank_score(const TokenizerPtr& tokenizer, const ModelPtr& model, const std::string &q, const std::string& a) {
        constexpr GenerationConfig config {.num_threads = 1};
        std::vector<int> ids;
        tokenizer->encode_qa(q, a, ids);
        return model->qa_rank(config, ids);
    }

    TEST(BGE_M3_RERANKER, test_encoder) {
        // model file is downlaoded during CMake configuration
        std::filesystem::path bge_m3_ranker_bin = std::filesystem::current_path() / "_assets/model_bins/bge-reranker-v2-m3.bin";
        ModelFactory model_factory;
        auto [ranker,tokenizer] = model_factory.load(bge_m3_ranker_bin.string());
        ASSERT_GT(get_rank_score(tokenizer, ranker, "hello", "welcome"), 0.9f);
    }
}

