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

    class DocumentIngestor {
    public:
        virtual ~DocumentIngestor()=default;
        virtual ResultIterator<Document>* Load(TextSplitter* text_splitter) = 0;

    };

}



#endif //DOCUMENTSOURCE_HPP
