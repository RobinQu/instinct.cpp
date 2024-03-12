//
// Created by RobinQu on 2024/3/12.
//

#ifndef IDOCSTORE_HPP
#define IDOCSTORE_HPP

#include "RetrievalGlobals.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    //
    // struct UpdateResult {
    //     int affected_rows;
    //     std::vector<std::string> retruned_ids;
    //     std::vector<Document> failed_documents;
    // };

    class IDocStore {
    public:
        IDocStore()=default;
        IDocStore(IDocStore&&)=delete;
        IDocStore(const IDocStore&)=delete;
        virtual ~IDocStore()=default;


        virtual void AddDocuments(const ResultIteratorPtr<Document>& documents_iterator, UpdateResult& update_result) = 0;

        /**
         * 
         * @param records Input records. ID will be updated after inserted.
         * @param update_result Update result containing failed documents and inserted IDs.
         */
        virtual void AddDocuments(std::vector<Document>& records, UpdateResult& update_result) = 0;

        /**
         * Add single document. Exception will be thrown directly.
         * @param doc Input record. ID will be updated after inserted.
         * @return 
         */
        virtual void AddDocument(Document& doc) = 0;


        virtual size_t DeleteDocuments(const std::vector<std::string>& ids) = 0;

        virtual ResultIteratorPtr<Document> MultiGetDocuments(const std::vector<std::string>& ids) = 0;

        [[nodiscard]] virtual const MetadataSchema& GetMetadataSchema() const = 0;
    };

    using DocStorePtr = std::shared_ptr<IDocStore>;
}

#endif //IDOCSTORE_HPP
