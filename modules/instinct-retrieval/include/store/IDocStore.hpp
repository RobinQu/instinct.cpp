//
// Created by RobinQu on 2024/3/12.
//

#ifndef IDOCSTORE_HPP
#define IDOCSTORE_HPP

#include "RetrievalGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    class IDocStore {
    public:
        IDocStore()=default;
        IDocStore(IDocStore&&)=delete;
        IDocStore(const IDocStore&)=delete;
        virtual ~IDocStore()=default;


        virtual void AddDocuments(const AsyncIterator<Document>& documents_iterator, UpdateResult& update_result) = 0;

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


        virtual void DeleteDocuments(const std::vector<std::string>& ids, UpdateResult& update_result) = 0;

        virtual AsyncIterator<Document> MultiGetDocuments(const std::vector<std::string>& ids) = 0;

        [[nodiscard]] virtual std::shared_ptr<MetadataSchema> GetMetadataSchema() const = 0;

        virtual size_t CountDocuments() = 0;
    };

    using DocStorePtr = std::shared_ptr<IDocStore>;
}

#endif //IDOCSTORE_HPP
