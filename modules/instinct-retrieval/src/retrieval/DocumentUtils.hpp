//
// Created by RobinQu on 2024/3/11.
//

#ifndef DOCUMENTUTILS_HPP
#define DOCUMENTUTILS_HPP


#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class DocumentUtils {
    public:
        static std::string CombineDocuments(const AsyncIterator<Document>& doc_itr) {
            std::string buf;
            doc_itr
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe([&](const Document& doc) {
                    buf += doc.text();
                    buf += "\n";
                })
            ;
            return buf;
        }
    };
}

#endif //DOCUMENTUTILS_HPP
