//
// Created by RobinQu on 2024/3/6.
//

#ifndef VECTORSTORERETRIEVER_HPP
#define VECTORSTORERETRIEVER_HPP


#include "BaseRetriever.hpp"
#include "store/IVectorStore.hpp"
#include "store/duckdb/DuckDBVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class VectorStoreRetriever final: public BaseStatefulRetriever {
        /**
         * vector_store_ will be used both as doc store and embedding store
         */
        VectorStorePtr vecstore_store_;

    public:
        explicit VectorStoreRetriever(
            VectorStorePtr vector_store)
            : vecstore_store_(std::move(vector_store)){
        }

        DocStorePtr GetDocStore() override {
            return vecstore_store_;
        }

        void Remove(const SearchQuery &metadata_query) override {
            UpdateResult update_result;
            vecstore_store_->DeleteDocuments(metadata_query, update_result);
            assert_true(update_result.failed_documents_size() == 0, "should have all documents deleted");
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest &search_request) const override {
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
        return std::make_shared<VectorStoreRetriever>(vector_store);
    }
}


#endif //VECTORSTORERETRIEVER_HPP
