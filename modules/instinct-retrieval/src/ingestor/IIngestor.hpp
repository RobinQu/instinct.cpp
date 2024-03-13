//
// Created by RobinQu on 2024/2/27.
//

#ifndef DOCUMENTSOURCE_HPP
#define DOCUMENTSOURCE_HPP

#include "RetrievalGlobals.hpp"
#include "document/TextSplitter.hpp"
#include "tools/ResultIterator.hpp"
#include <retrieval.pb.h>


namespace INSTINCT_RETRIEVAL_NS {

    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    class IIngestor {
    public:
        virtual ~IIngestor()=default;
        virtual ResultIteratorPtr<Document> Load() = 0;
        virtual ResultIteratorPtr<Document> LoadWithSpliter(const TextSplitterPtr& text_splitter) = 0;

    };

}



#endif //DOCUMENTSOURCE_HPP
