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

class OllamaEmbedding: public langchain::core::BaseEmbeddingModel {
    core::HttpRestClient client_;
    std::string model_name_;

public:
    OllamaEmbedding();
    OllamaEmbedding(core::Endpoint endpoint=OLLAMA_ENDPOINT, std::string model_name=OLLAMA_DEFUALT_MODEL_NAME);

    core::EmbeddingPtr EmbedDocuments(std::vector<std::string>& texts) override;

    core::EmbeddingPtr EmbedQuery(std::string& text) override;
};

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
            auto buf = new std::vector{std::move(response.embedding)};
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
        auto buf = new std::vector{std::move(response.embedding)};
        return std::make_shared<core::Embedding>{
            xt::adapt(buf, buf->size(), xt::acquire_ownership(), {buf->size()})
        };
    }

} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
