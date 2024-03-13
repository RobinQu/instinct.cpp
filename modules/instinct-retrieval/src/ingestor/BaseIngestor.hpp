//
// Created by RobinQu on 2024/3/13.
//

#ifndef BASEINGESTOR_HPP
#define BASEINGESTOR_HPP


#include "IIngestor.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace U_ICU_NAMESPACE;

    class BaseIngestor: public IIngestor {
    public:
        ResultIteratorPtr<Document> Load() override = 0;

        ResultIteratorPtr<Document> LoadWithSpliter(const TextSplitterPtr& text_splitter) override {
            const auto docs_itr = Load();
            return text_splitter->SplitDocuments(docs_itr);

        }
    };

    using IngestorPtr = std::shared_ptr<BaseIngestor>;
}

#endif //BASEINGESTOR_HPP
