//
// Created by RobinQu on 2024/2/28.
//

#ifndef EMBEDDINGS_HPP
#define EMBEDDINGS_HPP


#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    using Embedding = std::vector<float>;
    class IEmbeddingModel {
    public:
        IEmbeddingModel()=default;
        virtual ~IEmbeddingModel() = default;
        IEmbeddingModel(const IEmbeddingModel&)=delete;
        IEmbeddingModel(IEmbeddingModel&&)=delete;
        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) = 0;
        virtual Embedding EmbedQuery(const std::string& text) = 0;
        virtual size_t GetDimension() = 0;
    };

    using EmbeddingsPtr = std::shared_ptr<IEmbeddingModel>;
}

#endif //EMBEDDINGS_HPP
