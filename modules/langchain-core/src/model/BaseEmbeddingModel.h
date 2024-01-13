//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASEEMBBEDINGMODEL_H
#define BASEEMBBEDINGMODEL_H
#include <future>
#include <vector>

#include "../documents/Embedding.h"

namespace langchain {
namespace core {

class BaseEmbeddingModel {
    virtual ~BaseEmbeddingModel()=0;
    virtual EmbeddingPtr EmbedDocuments(std::vector<std::string>& texts) = 0;
    virtual EmbeddingPtr EmbedQuery(std::string& text) = 0;
    virtual std::future<EmbeddingPtr> EmbedDocumentsAsync(std::vector<std::string>& texts) = 0;
    virtual std::future<EmbeddingPtr> EmbedQueryAsync(std::string& text) = 0;
};

} // core
} // langchain

#endif //BASEEMBBEDINGMODEL_H
