//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include "RetrievalGlobals.hpp"
#include "store/Collection.hpp"
#include "store/SearchResult.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class DocumentRetrieval {

        SearchResult Retrieval(const std::string& query);
        [[nodiscard]] Collection& GetCllection() const;

    };

}

#endif //DOCUMENTRETRIEVAL_HPP
