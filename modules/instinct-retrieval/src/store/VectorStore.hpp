//
// Created by RobinQu on 2024/2/28.
//

#ifndef COLLECTIONSTORAGE_HPP
#define COLLECTIONSTORAGE_HPP


#include "Collection.hpp"
#include "RetrievalGlobals.hpp"
#include "model/BaseEmbeddingModel.hpp"
#include "model/Embeddings.hpp"


namespace INSTINCT_RETRIEVAL_NS {



    class VectorStore {
        virtual ~VectorStore() = default;

        [[nodiscard]] virtual Embeddings* GetEmbeddingModel() const = 0;


        virtual std::vector<Collection*> ShowCollections();
        [[nodiscard]] virtual Collection* GetCollection(const std::string& name) const = 0;
        virtual bool CreateCollection(const std::string& name, const DocumentSchema& schema) = 0;
        virtual bool DropCollection(const std::string&name)=0;


    };


}

#endif //COLLECTIONSTORAGE_HPP
