//
// Created by RobinQu on 2024/2/13.
//

#ifndef OLLAMAEMBEDDING_H
#define OLLAMAEMBEDDING_H
#include "ModelGlobals.h"
#include "commons/OllamaCommons.h"
#include "model/BaseEmbeddingModel.h"
#include "tools/HttpRestClient.h"


namespace LC_MODEL_NS {

class OllamaEmbedding: public core::BaseEmbeddingModel {
    core::HttpRestClient client_;
    std::string model_name_;

public:
    OllamaEmbedding();
    OllamaEmbedding(core::Endpoint endpoint=OLLAMA_ENDPOINT, std::string model_name=OLLAMA_DEFUALT_MODEL_NAME);

    core::EmbeddingPtr EmbedDocuments(std::vector<std::string>& texts) override;

    core::EmbeddingPtr EmbedQuery(std::string& text) override;
};

} // LC_MODEL_NS

#endif //OLLAMAEMBEDDING_H
