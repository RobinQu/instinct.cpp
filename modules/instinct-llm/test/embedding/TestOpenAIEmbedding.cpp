//
// Created by RobinQu on 2024/3/15.
//


#include <gtest/gtest.h>

#include <instinct/embedding_model/OpenAIEmbedding.hpp>


namespace INSTINCT_LLM_NS {
    /**
     * this test expects a local OpenAI-comppatible server is runnign at localhost:3928
     */
    class OpenAIEmbeddingTest: public testing::Test {
    protected:
        void SetUp() override {
            embedding_model_ = CreateOpenAIEmbeddingModel();
        }
        EmbeddingsPtr embedding_model_;
    };


    TEST_F(OpenAIEmbeddingTest, EmbedQuery) {
        auto result = embedding_model_->EmbedQuery("hello world");
        std::cout << result << std::endl;
    }

    TEST_F(OpenAIEmbeddingTest, EmbedDocuments) {
        const std::vector<std::string> texts = {
            R"(**(Advanced)** In case of implementing operator via lift you can control disposable strategy via updated_disposable_strategy parameter. It accepts disposable strategy of upstream and returns disposable strategy for downstream. It needed only for optimization and reducing disposables handling cost and it is purely advanced thing. Not sure if anyone is going to use it by its own for now =))",
            R"(It seems that the ContentProviderResourceReleaser is not being called when the socket connection is closed from the client side. In fact, the Response destructor is never called. Any ideas how to detect connection issues and cleanup resources?)"
        };
        const auto result = embedding_model_->EmbedDocuments(texts);
        ASSERT_EQ(result.size(), texts.size());
        for (const auto& item: result) {
            ASSERT_EQ(item.size(), embedding_model_->GetDimension());
            std::cout << item << std::endl;
        }
    }


}