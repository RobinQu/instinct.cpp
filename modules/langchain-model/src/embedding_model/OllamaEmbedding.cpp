//
// Created by RobinQu on 2024/2/13.
//

#include "OllamaEmbedding.h"
#include <xtensor/xadapt.hpp>

namespace LC_MODEL_NS {
    OllamaEmbedding::OllamaEmbedding(): client_(OLLAMA_ENDPOINT), model_name_(OLLAMA_DEFUALT_MODEL_NAME) {
    }

    OllamaEmbedding::OllamaEmbedding(core::Endpoint endpoint, std::string model_name): client_(std::move(endpoint)), model_name_(std::move(model_name)) {
    }


    core::EmbeddingPtr OllamaEmbedding::EmbedDocuments(std::vector<std::string>& texts) {
        auto embedding = std::make_shared<core::Embedding>();
        for(const auto& text: texts) {
            auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(OLLAMA_EMBEDDING_PATH, {
                model_name_,
                text,
                {}
            });
            auto shape = {1, response.embedding.size()};
            // move response to heap
            auto buf = new std::vector{response.embedding};
            // transfer to xarray
            auto one = xt::adapt(
                buf,
                buf->size(),
                xt::acquire_ownership(),
                shape
                );
            xt::stack(xt::xtuple(embedding->data, one));
        }
        return embedding;
    }

    core::EmbeddingPtr OllamaEmbedding::EmbedQuery(std::string& text) {
        auto response = client_.PostObject<OllamaEmbeddingRequest, OllamaEmbeddingResponse>(OLLAMA_EMBEDDING_PATH, {
                model_name_,
                text,
                {}
            });
        auto buf = new std::vector{response.embedding};
        return std::make_shared<core::Embedding>{
            xt::adapt(buf, buf->size(), xt::acquire_ownership(), {buf->size()})
        };
    }
} // LC_MODEL_NS