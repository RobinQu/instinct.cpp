//
// Created by RobinQu on 2024/3/11.
//

#ifndef IBATCHABLERETRIEVER_HPP
#define IBATCHABLERETRIEVER_HPP

#include "IRetriever.hpp"
#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using BacthedRetrievedDocuments = std::unordered_map<std::string, std::vector<Document>>;

    class IBatchableRetrieval: public IRetriever {
        virtual BacthedRetrievedDocuments Retrieve(const std::vector<std::string>& queries) = 0;
    };

    using BatchableRetrievalPtr = std::shared_ptr<IBatchableRetrieval>;

}



#endif //IBATCHABLERETRIEVER_HPP
