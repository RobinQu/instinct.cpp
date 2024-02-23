//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASEEMBBEDINGMODEL_H
#define BASEEMBBEDINGMODEL_H
#include <future>
#include <vector>

#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "LLMRuntimeOptions.hpp"
#include "chain/Chain.hpp"


LC_CORE_NS {
    using Embedding = std::vector<float>;

    class BaseEmbeddingModel : public Chain<std::string, Embedding, LLMRuntimeOptions> {
    public:
        // BaseEmbeddingModel()=default;
        // BaseEmbeddingModel(BaseEmbeddingModel&&)=delete;
        // BaseEmbeddingModel(const BaseEmbeddingModel&)=delete;
        // virtual ~BaseEmbeddingModel()=default;

        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts,
                                                      const LLMRuntimeOptions& options) = 0;


        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) = 0;

        virtual Embedding EmbedQuery(const std::string& text, const LLMRuntimeOptions& options) = 0;


        virtual Embedding EmbedQuery(const std::string& text) = 0;

        Embedding Invoke(const std::string& input, const LLMRuntimeOptions& options) override;

        std::vector<Embedding> Batch(const std::vector<std::string>& input, const LLMRuntimeOptions& options) override;

        std::vector<Embedding> Stream(const std::string& input, const LLMRuntimeOptions& options) override;
    };

    inline Embedding BaseEmbeddingModel::Invoke(const std::string& input, const LLMRuntimeOptions& options) {
        return EmbedQuery(input, options);
    }

    inline std::vector<Embedding> BaseEmbeddingModel::Batch(const std::vector<std::string>& input,
                                                            const LLMRuntimeOptions& options) {
        return EmbedDocuments(input, options);
    }

    inline std::vector<Embedding>
    BaseEmbeddingModel::Stream(const std::string& input, const LLMRuntimeOptions& options) {
        throw LangchainException("Not supported");
    }
} // core
// langchain

#endif //BASEEMBBEDINGMODEL_H
