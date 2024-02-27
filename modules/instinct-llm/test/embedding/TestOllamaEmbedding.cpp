//
// Created by RobinQu on 2024/2/23.
//
#include <gtest/gtest.h>


#include "embedding_model/OllamaEmbedding.hpp"
#include "tools/TensorUtils.hpp"
#include <string>
#include <iostream>


namespace INSTINCT_LLM_NS {


    TEST(OllamaEmbedding, TestSimpleOps) {
        using namespace INSTINCT_CORE_NS;
        OllamaEmbedding embedding;
        TensorUtils::PrintEmbedding(embedding.EmbedQuery("hell word"));

        std::vector<std::string> docs {
            R"""(Kazimierz Funk, commonly anglicized as Casimir Funk, was a Polish biochemist generally credited with being among the first to formulate the concept of vitamins, which he called "vital amines" or "vitamines")""",
            R"""(Albert Einstein was a German-born theoretical physicist who is widely held to be one of the greatest and most influential scientists of all time. )"""
        };

        for(const auto& vector: embedding.EmbedDocuments(docs)) {
            TensorUtils::PrintEmbedding(vector);
        }
    }

}