//
// Created by RobinQu on 2024/3/17.
//

#ifndef INSTINCT_BASERETRIEVER_HPP
#define INSTINCT_BASERETRIEVER_HPP

#include "retrieval/IRetriever.hpp"



namespace INSTINCT_RETRIEVAL_NS {

    class BaseRetriever: public ITextRetriever {
    public:
        AsyncIterator<Document> Retrieve(const TextQuery &query) override = 0;
    };
    using RetrieverPtr = std::shared_ptr<BaseRetriever>;

    class BaseStatefulRetriever: public BaseRetriever, public IStatefulRetriever {
    public:
        AsyncIterator<Document> Retrieve(const TextQuery &query) override = 0;
        void Ingest(const AsyncIterator<Document> &input) override = 0;
    };
    using StatefulRetrieverPtr = std::shared_ptr<BaseStatefulRetriever>;

}


#endif //INSTINCT_BASERETRIEVER_HPP
