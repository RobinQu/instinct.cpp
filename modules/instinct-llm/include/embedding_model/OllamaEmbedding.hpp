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
        OllamaConfiguration configuration_;

    public:
        explicit OllamaEmbedding(const OllamaConfiguration& configuration = {}):
            client_(configuration.endpoint),
            configuration_(configuration) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for (const auto& text: texts) {
                OllamaEmbeddingRequest request;
                request.set_model(configuration_.model_name);
                request.set_prompt(text);
                // TODO handle model options
                // request.mutable_options()->CopyFrom(configuration_->model_options());
                auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                    OLLAMA_EMBEDDING_PATH, request);
                result.emplace_back(response.embedding().begin(), response.embedding().end());
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OllamaEmbeddingRequest request;
                request.set_model(configuration_.model_name);
                request.set_prompt(text);
            // TODO handle model options
                // request.mutable_options()->CopyFrom(configuration_->model_options());
            const auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(
                OLLAMA_EMBEDDING_PATH, request);
            return {response.embedding().begin(), response.embedding().end()};
        }

        size_t GetDimension() override {
            // Ollama embedding cannot be configured with dimension
            // see https://github.com/ollama/ollama/issues/651
            return configuration_.dimension;
        }
    };


    static EmbeddingsPtr CreateOllamaEmbedding(const OllamaConfiguration& ollama_configuration = {}) {
        return std::make_shared<OllamaEmbedding>(ollama_configuration);
    }
} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
