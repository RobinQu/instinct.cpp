//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASEEMBBEDINGMODEL_H
#define BASEEMBBEDINGMODEL_H
#include <future>
#include <vector>

#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "chain/Chain.hpp"


namespace INSTINCT_CORE_NS {
    using Embedding = std::vector<float>;

    template<
        typename Configuration,
        typename RuntimeOptions
    >
    class BaseEmbeddingModel : public Chain<Configuration, RuntimeOptions, std::string, Embedding> {
    public:
        // BaseEmbeddingModel()=default;
        // BaseEmbeddingModel(BaseEmbeddingModel&&)=delete;
        // BaseEmbeddingModel(const BaseEmbeddingModel&)=delete;
        // virtual ~BaseEmbeddingModel()=default;

        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts,
                                                      const RuntimeOptions& options) = 0;


        virtual std::vector<Embedding> EmbedDocuments(const std::vector<std::string>& texts) = 0;

        virtual Embedding EmbedQuery(const std::string& text, const RuntimeOptions& options) = 0;


        virtual Embedding EmbedQuery(const std::string& text) = 0;

        Embedding Invoke(const std::string& input, const RuntimeOptions& options) override {
            return EmbedQuery(input, options);
        }

        Embedding Invoke(const std::string& input) override {
            return Invoke(input, {});
        }

        ResultIterator<Embedding>* Batch(const std::vector<std::string>& input) override {
            return Batch(input);
        }

        ResultIterator<Embedding>* Batch(const std::vector<std::string>& input, const RuntimeOptions& options) override {
            return create_from_range(EmbedDocuments(input, options));
        }

        ResultIterator<Embedding>* Stream(const std::string& input, const RuntimeOptions& options) override {
            return create_from_range(std::vector<Embedding>{Invoke(input, options)});
        }

        ResultIterator<Embedding>* Stream(const std::string& input) override {
            return Stream(input);
        }
    };


} // core
// langchain

#endif //BASEEMBBEDINGMODEL_H
