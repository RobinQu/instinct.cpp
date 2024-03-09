//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMAEMBEDDING_H
#define OLLAMAEMBEDDING_H
#include "LLMGlobals.hpp"
#include "commons/OllamaCommons.hpp"
#include "model/IEmbeddingModel.hpp"
#include "tools/HttpRestClient.hpp"
#include "llm.pb.h"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class OllamaEmbedding final : public IEmbeddingModel {
        HttpRestClient client_;
        std::shared_ptr<OllamaConfiguration> configuration_;

    public:
        explicit OllamaEmbedding(const std::shared_ptr<OllamaConfiguration>& configuration): client_(configuration->endpoint_host(), configuration->endpoint_port()), configuration_(configuration) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for (const auto& text: texts) {
                OllamaEmbeddingRequest request;
                request.set_model(configuration_->model_name());
                request.set_prompt(text);
                request.mutable_options()->CopyFrom(configuration_->model_options());
                auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                    OLLAMA_EMBEDDING_PATH, request);
                result.emplace_back(response.embedding().begin(), response.embedding().end());
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OllamaEmbeddingRequest request;
                request.set_model(configuration_->model_name());
                request.set_prompt(text);
                request.mutable_options()->CopyFrom(configuration_->model_options());
            const auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                OLLAMA_EMBEDDING_PATH, request);
            return {response.embedding().begin(), response.embedding().end()};
        }
    };
} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
