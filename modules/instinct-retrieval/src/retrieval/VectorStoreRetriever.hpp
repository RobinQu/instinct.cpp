//
// Created by RobinQu on 2024/3/6.
//

#ifndef VECTORSTORERETRIEVER_HPP
#define VECTORSTORERETRIEVER_HPP
#include "retrieval/IRetriever.hpp"
#include "store/VectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class VectorStoreRetriever: public IRetriever{
        VectoreStorePtr vectore_store_;
        SearchRequest search_request_template_;

    public:
        explicit VectorStoreRetriever(VectoreStorePtr vectore_store, const SearchRequest& search_request_template)
            : vectore_store_(std::move(vectore_store)), search_request_template_(search_request_template){
        }

        ResultIterator<Document>* Retrieve(const std::string& query) override {
            SearchRequest search_request;
            search_request.CopyFrom(search_request_template_);
            search_request.set_query(query);
            return vectore_store_->SearchDocuments(search_request);
        }
    };
}


#endif //VECTORSTORERETRIEVER_HPP
