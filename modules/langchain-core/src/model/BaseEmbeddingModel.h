//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASEEMBBEDINGMODEL_H
#define BASEEMBBEDINGMODEL_H
#include <future>
#include <vector>

#include "../document/Embedding.h"
#include "CoreGlobals.h"


namespace LC_CORE_NS {

class BaseEmbeddingModel {
public:
    BaseEmbeddingModel()=default;
    BaseEmbeddingModel(BaseEmbeddingModel&&)=delete;
    BaseEmbeddingModel(const BaseEmbeddingModel&)=delete;
    virtual ~BaseEmbeddingModel()=0;

    virtual EmbeddingPtr EmbedDocuments(std::vector<std::string>& texts) = 0;
    virtual EmbeddingPtr EmbedQuery(std::string& text) = 0;
    // virtual std::future<EmbeddingPtr> EmbedDocumentsAsync(std::vector<std::string>& texts) = 0;
    // virtual std::future<EmbeddingPtr> EmbedQueryAsync(std::string& text) = 0;
};

} // core
// langchain

#endif //BASEEMBBEDINGMODEL_H
