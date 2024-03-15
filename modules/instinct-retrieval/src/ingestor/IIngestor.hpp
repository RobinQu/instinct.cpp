//
// Created by RobinQu on 2024/2/27.
//

#ifndef DOCUMENTSOURCE_HPP
#define DOCUMENTSOURCE_HPP

#include "RetrievalGlobals.hpp"
#include "document/TextSplitter.hpp"
#include <retrieval.pb.h>


namespace INSTINCT_RETRIEVAL_NS {

    using namespace INSTINCT_CORE_NS;
    using namespace INSTINCT_LLM_NS;

    /**
    * Interface for datasource of documents
    */
    class IIngestor {
    public:
        virtual ~IIngestor()=default;

        /**
         *
         * @return Iterator of loaded documents
         */
        virtual AsyncIterator<Document> Load() = 0;

        /**
         * Split loaded documents which are used to create chunked documents
         * @param text_splitter spliiter to be used for each loaded document
         * @return
         */
        virtual AsyncIterator<Document> LoadWithSplitter(const TextSplitterPtr& text_splitter) = 0;

    };

}



#endif //DOCUMENTSOURCE_HPP
