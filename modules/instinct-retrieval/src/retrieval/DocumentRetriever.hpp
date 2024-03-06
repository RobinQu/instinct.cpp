//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include "RetrievalGlobals.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class DocumentRetriever {
    public:
        DocumentRetriever()=default;
        virtual ~DocumentRetriever() = default;
        DocumentRetriever(const DocumentRetriever&)=delete;
        DocumentRetriever(DocumentRetriever&&)=delete;

        virtual ResultIterator<Document>* Retrieve(const std::string& query) = 0;
    };

}

#endif //DOCUMENTRETRIEVAL_HPP
