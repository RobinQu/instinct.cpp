//
// Created by RobinQu on 2024/5/26.
//


#include <gtest/gtest.h>
#include <filesystem>
#include <thread>

#include <instinct/transformer/model_factory.hpp>
#include <instinct/core_global.hpp>

namespace INSTINCT_TRANSFORMER_NS {
    using namespace INSTINCT_CORE_NS;
    class BGEM3RankerTest: public testing::Test {
    protected:
        const std::filesystem::path bge_m3_ranker_bin = std::filesystem::current_path() / "_assets/model_bins/bge-reranker-v2-m3.bin";

        ModelFactory model_factory;
        TokenizerPtr tokenizer_;
        ModelPtr model_;

        void SetUp() override {
            SetupLogging();
            load_bge_m3_ranker();
            std::cout << print_system_info() << std::endl;
        }

        void load_bge_m3_ranker() {
            std::tie(this->model_, this->tokenizer_) = model_factory.load(this->bge_m3_ranker_bin.string());
        }

        [[nodiscard]] float get_rank_score(const std::string &q, const std::string& a) const {
            const GenerationConfig config {.num_threads = std::thread::hardware_concurrency() / 2};
            std::vector<int> ids;
            this->tokenizer_->encode_qa(q, a, ids);
            LOG_INFO("token size {}", ids.size());
            const auto t1 = std::chrono::system_clock::now();
            const auto score =  this->model_->qa_rank(config, ids);
            std::cout << "num_thread: " << config.num_threads << ", elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-t1).count() << std::endl;
            return score;
        }
    };

    TEST_F(BGEM3RankerTest, test_encoder) {
        ASSERT_EQ(
            tokenizer_->encode("hello"),
            std::vector({0, 33600, 31, 2})
        );
    }

    TEST_F(BGEM3RankerTest, test_ranker) {
        ASSERT_GT(get_rank_score("hello", "hello"), 0.7f);
        // ASSERT_LT(get_rank_score("hello", "farewell"), 0.1);
    }

    TEST_F(BGEM3RankerTest, test_long_text) {
        const auto t1 = std::chrono::system_clock::now();
        ASSERT_GT(get_rank_score("hello", R"(Create an Endpoint\n\nAfter your first login, you will be directed to the [Endpoint creation page](https://ui.endpoints.huggingface.co/new). As an example, this guide will go through the steps to deploy [distilbert-base-uncased-finetuned-sst-2-english](https://huggingface.co/distilbert-base-uncased-finetuned-sst-2-english) for text classification. \n\n## 1. Enter the Hugging Face Repository ID and your desired endpoint name:\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_repository.png\" alt=\"select repository\" />",
      "## 2. Select your Cloud Provider and region. Initially, only AWS will be available as a Cloud Provider with the `us-east-1` and `eu-west-1` regions. We will add Azure soon, and if you need to test Endpoints with other Cloud Providers or regions, please let us know.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_region.png\" alt=\"select region\" />\n\n## 3. Define the [Security Level](security) for the Endpoint:\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_security.png\" alt=\"define security\" />",
      "## 4. Create your Endpoint by clicking **Create Endpoint**. By default, your Endpoint is created with a medium CPU (2 x 4GB vCPUs with Intel Xeon Ice Lake) The cost estimate assumes the Endpoint will be up for an entire month, and does not take autoscaling into account.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/1_create_cost.png\" alt=\"create endpoint\" />\n\n## 5. Wait for the Endpoint to build, initialize and run which can take between 1 to 5 minutes.\n\n<img src=\"https://raw.githubusercontent.com/huggingface/hf-endpoints-documentation/main/assets/overview.png\" alt=\"overview\" />\n\n## 6. Test your Endpoint in the overview with the Inference widget \ud83c\udfc1 \ud83c\udf89!)"), 0);
        const auto t2 = std::chrono::system_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() <<std::endl;
    }



}

