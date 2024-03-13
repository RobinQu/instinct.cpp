//
// Created by RobinQu on 2024/3/6.
//

#ifndef VECTORSTORERETRIEVER_HPP
#define VECTORSTORERETRIEVER_HPP
#include <utility>

#include "IStatefulRetriever.hpp"
#include "retrieval/IRetriever.hpp"
#include "store/IVectorStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class VectorStoreRetriever: public IStatefulRetriever<TextQuery> {
        /**
         * vector_store_ will be used both as doc store and embedding store
         */
        VectorStorePtr vectore_store_;
        SearchRequest search_request_template_;

    public:
        explicit VectorStoreRetriever(VectorStorePtr vectore_store, SearchRequest search_request_template = {})
            : vectore_store_(std::move(vectore_store)), search_request_template_(std::move(search_request_template)){
        }

        static StatefulRetrieverPtr Create(const DuckDbVectoreStoreOptions& options) {
            auto store = std::make_shared<DuckDBVectorStore>(options);
            return std::make_shared<VectorStoreRetriever>(store);
        }

        ResultIteratorPtr<Document> Retrieve(const TextQuery& query) override {
            SearchRequest search_request;
            search_request.MergeFrom(search_request_template_);
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            return vectore_store_->SearchDocuments(search_request);
        }

        void Ingest(const ResultIteratorPtr<Document>& input) override {
            UpdateResult update_result;
            vectore_store_->AddDocuments(input, update_result);
            assert_true(update_result.failed_documents_size() == 0, "should not have failed documents");
        }
    };
}


#endif //VECTORSTORERETRIEVER_HPP
