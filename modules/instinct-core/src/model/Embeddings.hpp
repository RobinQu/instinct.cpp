//
// Created by RobinQu on 2024/2/28.
//

#ifndef EMBEDDINGS_HPP
#define EMBEDDINGS_HPP

#include "BaseEmbeddingModel.hpp"
#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    class Embeddings {
    public:
        virtual ~Embeddings() = default;
        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) = 0;
        virtual Embedding EmbedQuery(const std::string& text) = 0;
    };

    using EmbeddingsPtr = std::shared_ptr<Embeddings>;
}

#endif //EMBEDDINGS_HPP
