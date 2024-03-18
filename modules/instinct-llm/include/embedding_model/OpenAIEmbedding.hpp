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
            assert_gt(configuration.dimension, 0, "dimension should be greater than zero");
        }

        std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) override {
            assert_lte(texts.size(), 2048, "Number of given texts should be less than 2048");
            std::vector<Embedding> result;
            result.reserve(texts.size());

            OpenAIEmbeddingRequest req;
            for(const auto& text: texts) {
                req.add_input(text);
            }
            req.set_model(configuration_.model_name);
            req.set_dimension(configuration_.dimension);
            auto res = client_.PostObject<OpenAIEmbeddingRequest, OpenAIEmbeddingResponse>(DEFAULT_OPENAI_EMBEDDING_ENDPOINT, req);

            assert_true(res.data_size()>0, "should have at least one embedding returned");
            for(const auto&embedding_response: res.data()) {
                Embedding embedding;
                embedding.reserve(configuration_.dimension);
                for (const auto& f: embedding_response.embedding()) {
                    embedding.push_back(f);
                }
                result.push_back(embedding);
            }
            return result;
        }

        Embedding EmbedQuery(const std::string& text) override {
            OpenAIEmbeddingRequest req;
            req.add_input(text);
            req.set_model(configuration_.model_name);
            req.set_dimension(configuration_.dimension);
            auto res = client_.PostObject<OpenAIEmbeddingRequest, OpenAIEmbeddingResponse>(DEFAULT_OPENAI_EMBEDDING_ENDPOINT, req);
            Embedding embedding;
            embedding.reserve(configuration_.dimension);
            assert_true(res.data_size()>0, "should have at least one embedding returned");
            for(const auto&f: res.data(0).embedding()) {
                embedding.push_back(f);
            }
            return embedding;
        }

        size_t GetDimension() override {
            return configuration_.dimension;
        }
    };

    static EmbeddingsPtr CreateOpenAIEmbeddingModel(const OpenAIConfiguration& configuration_) {
        return std::make_shared<OpenAIEmbedding>(configuration_);
    }

}

#endif //OPENAIEMBEDDING_HPP
