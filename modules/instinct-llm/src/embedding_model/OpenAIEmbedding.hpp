//
// Created by RobinQu on 2024/3/15.
//

#ifndef OPENAIEMBEDDING_HPP
#define OPENAIEMBEDDING_HPP

#include <model/IEmbeddingModel.hpp>

#include "LLMGlobals.hpp"
#include "OllamaEmbedding.hpp"
#include "commons/OpenAICommons.hpp"
#include "tools/HttpRestClient.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class OpenAIEmbedding final: public IEmbeddingModel {
        OpenAIConfiguration configuration_;
        HttpRestClient client_;

    public:
        explicit OpenAIEmbedding(OpenAIConfiguration configuration)
            : configuration_(std::move(configuration)), client_(configuration_.endpoint) {
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            std::vector<Embedding> result;
            for (const auto& text: texts) {
                result.push_back(EmbedQuery(text));
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OpenAIEmbeddingRequest req;
            req.set_input(text);
            req.set_model(configuration_.model_name);
            req.set_dimension(configuration_.dimension);
            auto res = client_.PostObject<OpenAIEmbeddingRequest, OpenAIEmbeddingResponse>(DEFAULT_OPENAI_EMBEDDING_ENDPOINT, req);
            Embedding embedding;
            embedding.reserve(configuration_.dimension);
            for(const auto&f: res.embedding()) {
                embedding.push_back(f);
            }
            return embedding;
        }

    };
}

#endif //OPENAIEMBEDDING_HPP
