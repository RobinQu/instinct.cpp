//
// Created by RobinQu on 2024/2/28.
//

#ifndef COLLECTIONSTORAGE_HPP
#define COLLECTIONSTORAGE_HPP

#include <retrieval.pb.h>
#include "RetrievalGlobals.hpp"
#include "model/BaseEmbeddingModel.hpp"
#include "model/IEmbeddingModel.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    /**
     * Interface for document store. Forgive the poor name of `VectorStore`, which is follows convention in langchain, etc, but actually misleading actually in two ways:
     *
     * 1. It's only an abstraction for data access operations on a single table (or a collection, or a schema, or whatever you name a container of rows), not for database-level actions.
     * 2. Document normally contains both scalar fields like int, string and vector field. But many still call it `Vector` document.
     *
     * Building a full-fledged hybrid vector store isn't my primary goal, so `VectoreStore` classes will follow practices in other similar LLM tools.
     */
    class VectorStore {
    public:
        VectorStore()=default;
        virtual ~VectorStore() = default;
        VectorStore(VectorStore&&)=delete;
        VectorStore(const VectorStore&)=delete;

        // [[nodiscard]] virtual Embeddings* GetEmbeddingModel() const = 0;
        virtual std::vector<std::string> AddDocuments(ResultIterator<Document>* documents_iterator) = 0;
        virtual void AddDocuments(std::vector<Document>& records, std::vector<std::string>& id_result) = 0;
        virtual size_t DeleteDocuments(const std::vector<std::string>& ids) = 0;
        virtual ResultIterator<Document>* SearchDocuments(const SearchRequest& request) = 0;
    };

    using VectoreStorePtr = std::shared_ptr<VectorStore>;


}

#endif //COLLECTIONSTORAGE_HPP
