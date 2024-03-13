//
// Created by RobinQu on 2024/3/12.
//

#ifndef ISTATEFULRETRIEVER_HPP
#define ISTATEFULRETRIEVER_HPP

#include "RetrievalGlobals.hpp"
#include "retrieval/IRetriever.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    struct IngestionResult {
        std::vector<std::string> inserted_ids;
        ResultIteratorPtr<Document> failed_docs;
    };

    template<typename T>
    class IStatefulRetriever: public IRetriever<T> {
    public:
        virtual void Ingest(const ResultIteratorPtr<Document>& input) = 0;
    };

    using StatefulRetrieverPtr = std::shared_ptr<IStatefulRetriever<TextQuery>>;
}

#endif //ISTATEFULRETRIEVER_HPP
