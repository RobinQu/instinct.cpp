//
// Created by RobinQu on 2024/5/28.
//

#ifndef RETRIEVERFACTORY_HPP
#define RETRIEVERFACTORY_HPP

#include <document/RecursiveCharacterTextSplitter.hpp>

#include "AssistantGlobals.hpp"
#include "ranker/LocalRankingModel.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/ChunkedMultiVectorRetriever.hpp"
#include "retrieval/MutliPathRetriever.hpp"
#include "store/IVectorStoreOperator.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"


namespace INSTINCT_ASSISTANT_NS::v2 {
    using namespace INSTINCT_RETRIEVAL_NS;

    struct RetrieverOperatorOptions {
        int parent_chunk_size = 0;
        int child_chunk_size = 800;
        int child_chunk_overlap = 400;
    };

    static const std::string VECTOR_STORE_FILE_ID_KEY = "file-id";
    static const std::string VECTOR_STORE_ID_KEY = "vs-id";

    /**
     * This class encapsulate how retriever is built with related strategies.
     *
     * 1. It uses shared doc store for all retrievers. This could be improved in the future when scalability is vital.
     * 2. It uses MultiPathRetriever with local ranker and CreateChunkedMultiVectorRetriever as its child retrievers.
     *
     */
    class RetrieverOperator {
        VectorStoreOperatorPtr vector_store_operator_;
        DocStorePtr doc_store_;
        RetrieverOperatorOptions options_;
        MetadataSchemaPtr metadata_schema_;

    public:
        RetrieverOperator(
            VectorStoreOperatorPtr vector_store_operator,
            DocStorePtr doc_store,
            const RetrieverOperatorOptions& options = {})
            : vector_store_operator_(std::move(vector_store_operator)),
              doc_store_(std::move(doc_store)),
              options_(options) {
            assert_true(options_.parent_chunk_size > options_.child_chunk_size, "parent_chunk_size should be larger than child_chunk_size");
        }

        [[nodiscard]] VectorStorePtr ProvisionRetriever(const VectorStoreObject& vector_store_object) const {
            return vector_store_operator_->CreateInstance(vector_store_object.id(), metadata_schema_);
        }

        [[nodiscard]] bool CleanupRetriever(const VectorStoreObject& vector_store_object) const {
            SearchQuery search_query;
            search_query.mutable_term()->set_name(VECTOR_STORE_ID_KEY);
            search_query.mutable_term()->mutable_term()->set_string_value(vector_store_object.id());
            UpdateResult update_result;
            doc_store_->DeleteDocuments(search_query, update_result);
            if (update_result.failed_documents_size() == 0) {
                return vector_store_operator_->RemoveInstance(vector_store_object.id());
            }
            LOG_WARN("Failed to cleanup documents for VectorStoreObject {}", vector_store_object.ShortDebugString());
            return false;
        }

        /**
         * Return the readonly retriever
         * @param vector_store_object
         * @return
         */
        [[nodiscard]] RetrieverPtr GetStatelessRetriever(const VectorStoreObject& vector_store_object) const {
            const auto vector_store = vector_store_operator_->LoadInstance(vector_store_object.id());
            const auto ranking_model = CreateLocalRankingModel(BGE_M3_RERANKER);
            const auto child_retriever = GetStatefulRetriever(vector_store_object);
            return CreateMultiPathRetriever(ranking_model, child_retriever);
        }


        /**
         * Return writable retriever that accept document ingestion
         * @param vector_store_object
         * @return
         */
        [[nodiscard]] StatefulRetrieverPtr GetStatefulRetriever(const VectorStoreObject& vector_store_object) const {
            const auto vector_store = vector_store_operator_->LoadInstance(vector_store_object.id());
            const auto tokenizer = TiktokenTokenizer::MakeGPT4Tokenizer();
            const auto child_spliter = CreateRecursiveCharacterTextSplitter(tokenizer, {
                .chunk_size = options_.child_chunk_size,
                .chunk_overlap = options_.child_chunk_overlap
            });
            if (options_.parent_chunk_size > 0) {
                const auto parent_splitter = CreateRecursiveCharacterTextSplitter({.chunk_size = options_.parent_chunk_size});
                return  CreateChunkedMultiVectorRetriever(
                    doc_store_,
                    vector_store,
                    child_spliter,
                    parent_splitter
                );
            }
            return  CreateChunkedMultiVectorRetriever(
                doc_store_,
                vector_store,
                child_spliter
            );
        }
    };


    using RetrieverOperatorPtr = std::shared_ptr<RetrieverOperator>;
}


#endif //RETRIEVERFACTORY_HPP
