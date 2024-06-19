//
// Created by RobinQu on 2024/6/17.
//

#include <filesystem>
#include <thread>
#include <gtest/gtest.h>

#include "models//bge_embedding.hpp"
#include "model_factory.hpp"
#include "tools/TensorUtils.hpp"

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

        std::vector<float> get_embedding(const std::string& text) {
            const GenerationConfig config {.num_threads = std::thread::hardware_concurrency()};
            const auto tokens = tokenizer_->encode("hello");
            std::vector<float> embedding;
            model_->text_embedding(config, tokens, embedding);
            return embedding;
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
        core::TensorUtils::PrintEmbedding(embedding);
    }

    TEST_F(BGEM3EmbeddingTest, test_long_text) {
        const auto result = get_embedding(R"(Create an Endpoint\n\nAfter your first login, you will be directed to the [Endpoint creation page](https://ui.endpoints.huggingface.co/new). As an example, this guide will go through the steps to deploy [distilbert-base-uncased-finetuned-sst-2-english](https://huggingface.co/distilbert-base-uncased-finetuned-sst-2-english) for text classification. \n\n## 1. Enter the Hugging Face Repository ID and your desired endpoint name:\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_repository.png\" alt=\"select repository\" />",
      "## 2. Select your Cloud Provider and region. Initially, only AWS will be available as a Cloud Provider with the `us-east-1` and `eu-west-1` regions. We will add Azure soon, and if you need to test Endpoints with other Cloud Providers or regions, please let us know.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_region.png\" alt=\"select region\" />\n\n## 3. Define the [Security Level](security) for the Endpoint:\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_security.png\" alt=\"define security\" />",
      "## 4. Create your Endpoint by clicking **Create Endpoint**. By default, your Endpoint is created with a medium CPU (2 x 4GB vCPUs with Intel Xeon Ice Lake) The cost estimate assumes the Endpoint will be up for an entire month, and does not take autoscaling into account.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_create_cost.png\" alt=\"create endpoint\" />\n\n## 5. Wait for the Endpoint to build, initialize and run which can take between 1 to 5 minutes.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/overview.png\" alt=\"overview\" />\n\n## 6. Test your Endpoint in the overview with the Inference widget \ud83c\udfc1 \ud83c\udf89!)");
      core::TensorUtils::PrintEmbedding(result);

    }
}
