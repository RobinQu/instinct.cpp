//
// Created by RobinQu on 2024/3/6.
//

#ifndef VECTORSTORERETRIEVER_HPP
#define VECTORSTORERETRIEVER_HPP
#include <utility>


#include "BaseRetriever.hpp"
#include "store/IVectorStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class VectorStoreRetriever: public BaseStatefulRetriever {
        /**
         * vector_store_ will be used both as doc store and embedding store
         */
        VectorStorePtr vecstore_store_;

        /**
         * Template object that every search request objects will copied from
         */
        std::shared_ptr<SearchRequest> search_request_template_;

    public:
        explicit VectorStoreRetriever(
            VectorStorePtr vectore_store,
            std::shared_ptr<SearchRequest> search_request_template)
            : vecstore_store_(std::move(vectore_store)), search_request_template_(std::move(search_request_template)){
        }


        AsyncIterator<Document> Retrieve(const TextQuery& query) override {
            SearchRequest search_request;
            if (search_request_template_) {
                search_request.MergeFrom(*search_request_template_);
            }
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            return vecstore_store_->SearchDocuments(search_request);
        }

        void Ingest(const AsyncIterator<Document>& input) override {
            UpdateResult update_result;
            vecstore_store_->AddDocuments(input, update_result);
            LOG_DEBUG("Ingest done, added={}, failed={}", update_result.affected_rows(), update_result.failed_documents().size());
            assert_true(update_result.failed_documents_size() == 0, "should not have failed documents");
        }
    };

    static StatefulRetrieverPtr CreateVectorStoreRetriever(const VectorStorePtr& vector_store) {
        return std::make_shared<VectorStoreRetriever>(vector_store, nullptr);
    }
}


#endif //VECTORSTORERETRIEVER_HPP
