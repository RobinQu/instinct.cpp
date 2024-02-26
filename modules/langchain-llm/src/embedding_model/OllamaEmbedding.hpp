//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMAEMBEDDING_H
#define OLLAMAEMBEDDING_H
#include "ModelGlobals.hpp"
#include "commons/OllamaCommons.hpp"
#include "model/BaseEmbeddingModel.hpp"
#include "tools/HttpRestClient.hpp"

LC_LLM_NS {
    class OllamaEmbedding : public langchain::core::BaseEmbeddingModel<OllamaConfiguration, OllamaRuntimeOptions> {
        core::HttpRestClient client_;

    public:
        explicit OllamaEmbedding(core::Endpoint endpoint = OLLAMA_ENDPOINT): client_(std::move(endpoint)) {
        }

        std::vector<core::Embedding> EmbedDocuments(const std::vector<std::string>& texts,
                                                    const OllamaRuntimeOptions& options) override;

        core::Embedding EmbedQuery(const std::string& text, const OllamaRuntimeOptions& options) override;

        std::vector<core::Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            return EmbedDocuments(texts, {});
        }

        core::Embedding EmbedQuery(const std::string& text) override {
            return EmbedQuery(text, {});
        }
    };

    inline std::vector<core::Embedding> OllamaEmbedding::EmbedDocuments(const std::vector<std::string>& texts,
                                                                        const OllamaRuntimeOptions& options) {
        std::vector<core::Embedding> result;
        for (const auto& text: texts) {
            auto&& [embedding] = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                OLLAMA_EMBEDDING_PATH, {
                    options.model_name,
                    text,
                    {}
                });
            result.emplace_back(std::move(embedding));
        }
        return result;
    }

    inline core::Embedding
    OllamaEmbedding::EmbedQuery(const std::string& text, const OllamaRuntimeOptions& options) {
        auto&& [embedding] = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
            OLLAMA_EMBEDDING_PATH, {
                options.model_name,
                text,
                {}
            });
        return embedding;
    }
} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
