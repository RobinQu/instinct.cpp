//
// Created by RobinQu on 2024/3/15.
//

#ifndef LLMTESTGLOBALS_HPP
#define LLMTESTGLOBALS_HPP
#include <random>

#include "RetrievalGlobals.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "chat_model/OpenAIChat.hpp"
#include "embedding_model/OpenAIEmbedding.hpp"
#include "llm/BaseLLM.hpp"
#include "llm/OpenAILLM.hpp"
#include "tools/ChronoUtils.hpp"

namespace INSTINCT_LLM_NS::test {
    using namespace INSTINCT_CORE_NS;


    static Embedding make_zero_vector(const size_t dim=128) {
        Embedding embedding (dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(0);
        }
        return embedding;
    }

    static Embedding make_random_vector(const size_t dim=128) {
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<float> dis(0, 1.0);

        Embedding embedding;
        embedding.reserve(dim);
        for(size_t i=0;i<dim;i++) {
            embedding.push_back(dis(gen));
        }
        return embedding;
    }

    class PesudoChatModel : public BaseChatModel {
    public:
        explicit PesudoChatModel(const ModelOptions &options = {}) : BaseChatModel(options) {}

    private:
        BatchedLangaugeModelResult Generate(const std::vector<MessageList> &messages) override {
            BatchedLangaugeModelResult batched_model_result;
            for(const auto& message_list: messages) {
                auto* result = batched_model_result.add_generations();
                auto* gen =result->add_generations();
                gen->set_text("talking non-sense");
                auto* msg = gen->mutable_message();
                msg->set_content(R"(
talking non-sense
talking non-sense
talking non-sense

talking non-sense
)");
                msg->set_role("assistant");
            }
            return batched_model_result;
        }

        AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList &messages) override {
            LangaugeModelResult model_result;
            return rpp::source::just(model_result);
        }


    };


    class PesuodoEmbeddings final: public IEmbeddingModel {
        std::unordered_map<std::string, Embedding> caches_ = {};
        size_t dim_;
    public:
        explicit PesuodoEmbeddings(const size_t dim = 512)
                : dim_(dim) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for(const auto& text: texts) {
                if (!caches_.contains(text)) {
                    caches_.emplace(text, make_random_vector(dim_));
                }
                result.push_back(caches_.at(text));
            }
            return result;
        }

        [[nodiscard]] auto& get_caches()  const {
            return caches_;
        }

        size_t GetDimension() override {
            return dim_;
        }


        Embedding EmbedQuery(const std::string& text) override {
            if (!caches_.contains(text)) {
                caches_.emplace(text, make_random_vector(dim_));
            }
            return caches_.at(text);
        }
    };

    static ChatModelPtr create_pesudo_chat_model() {
        return std::make_shared<PesudoChatModel>();
    }

    static std::shared_ptr<PesuodoEmbeddings> create_pesudo_embedding_model(size_t dim = 512) {
        return std::make_shared<PesuodoEmbeddings>(dim);
    }

    static OpenAIConfiguration DEFAULT_NITRO_SERVER_CONFIGURATION = {
        .endpoint = {.host = "localhost", .port = 3928},
        .model_name = "local-model",
        .dimension = 512
    };

    static std::filesystem::path ensure_random_temp_folder() {
        auto root_path = std::filesystem::temp_directory_path() / "instinct-test" / std::to_string(
                             ChronoUtils::GetCurrentTimeMillis());
        std::filesystem::create_directories(root_path);
        return root_path;
    }

    static ChatModelPtr create_local_chat_model() {
        return CreateOpenAIChatModel(DEFAULT_NITRO_SERVER_CONFIGURATION);
    }

    static LLMPtr create_local_llm() {
        return CreateOpenAILLM(DEFAULT_NITRO_SERVER_CONFIGURATION);
    }

    static EmbeddingsPtr create_local_embedding_model(const size_t dimension = 512) {
        auto conf = DEFAULT_NITRO_SERVER_CONFIGURATION;
        conf.dimension = dimension;
        return CreateOpenAIEmbeddingModel(conf);
    }
}


#endif //LLMTESTGLOBALS_HPP
