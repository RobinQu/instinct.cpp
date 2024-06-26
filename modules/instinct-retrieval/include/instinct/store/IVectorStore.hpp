//
// Created by RobinQu on 2024/2/28.
//

#ifndef COLLECTIONSTORAGE_HPP
#define COLLECTIONSTORAGE_HPP

#include <retrieval.pb.h>
#include <instinct/RetrievalGlobals.hpp>
#include <instinct/model/embedding_model.hpp>
#include <instinct/store/IDocStore.hpp>
#include <instinct/tools/metadata_schema_builder.hpp>


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    /**
     * Interface for document store. Forgive the poor name of `VectorStore`, which is follows convention in langchain, etc., but actually misleading in two ways:
     *
     * 1. It's only an abstraction for data access operations on a single table (or a collection, or a schema, or whatever you name a container of rows), not for database-level actions.
     * 2. Document normally contains both scalar fields like int, string and vector field. But many still call it `Vector` document.
     *
     * Building a full-fledged hybrid vector store isn't my primary goal, so `VectorStore` classes will follow practices in other similar LLM tools.
     */
    class IVectorStore: public IDocStore {
    public:
        virtual AsyncIterator<Document> SearchDocuments(const SearchRequest& request) = 0;
        virtual EmbeddingsPtr GetEmbeddingModel() = 0;
    };
    using VectorStorePtr = std::shared_ptr<IVectorStore>;

}

#endif //COLLECTIONSTORAGE_HPP
