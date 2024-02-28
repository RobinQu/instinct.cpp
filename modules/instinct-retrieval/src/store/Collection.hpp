//
// Created by RobinQu on 2024/2/27.
//

#ifndef VECTORSTORE_HPP
#define VECTORSTORE_HPP

#include "Record.hpp"
#include "RetrievalGlobals.hpp"
#include "document/Document.hpp"
#include "model/BaseEmbeddingModel.hpp"
#include "tools/ResultIterator.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    struct MetadataFilter {
        std::vector<Field> must;
        std::vector<Field> mustNot;
    };

    struct SearchOptions {
        int top_k = 10;
        MetadataFilter filter;
    };

    class Collection {
    public:
        virtual ~Collection() = default;
        [[nodiscard]] virtual DocumentSchema& GetSchema() const=0;
        virtual std::vector<std::string> AddDocuments(ResultIterator<Document>* documents_iterator) = 0;
        virtual std::vector<std::string> AddDocuments(const std::vector<Document>& records) = 0;
        virtual bool DeleteDocuments(const std::vector<std::string>& ids) = 0;
        virtual ResultIterator<Document>* SearchDocuments(const std::string& query, const SearchOptions& options) = 0;
    };

}


#endif //VECTORSTORE_HPP
