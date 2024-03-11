//
// Created by RobinQu on 2024/3/11.
//

#ifndef DOCUMENTUTILS_HPP
#define DOCUMENTUTILS_HPP


#include "RetrievalGlobals.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class DocumentUtils {
    public:
        static std::string CombineDocuments(const ResultIteratorPtr<Document>& doc_itr) {
            std::string buf;
            while (doc_itr->HasNext()) {

            }
            return buf;

        }
    };
}

#endif //DOCUMENTUTILS_HPP
