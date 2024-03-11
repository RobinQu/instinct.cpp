//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include "RetrievalGlobals.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class IRetriever {
    public:
        IRetriever()=default;
        virtual ~IRetriever() = default;
        IRetriever(const IRetriever&)=delete;
        IRetriever(IRetriever&&)=delete;

        virtual ResultIteratorPtr<Document> Retrieve(const std::string& query) = 0;
    };

    using RetrieverPtr = std::shared_ptr<IRetriever>;

}

#endif //DOCUMENTRETRIEVAL_HPP
