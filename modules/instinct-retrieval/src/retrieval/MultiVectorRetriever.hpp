//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIVECTORRETRIEVER_HPP
#define MULTIVECTORRETRIEVER_HPP


#include "IStatefulRetriever.hpp"
#include "chain/LLMChain.hpp"
#include "store/IVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    // class MultiVectorGuidance {
    // public:
    //     virtual ~MultiVectorGuidance() = default;
    //     virtual ResultIteratorPtr<Document> GenerateGuidanceDocs(const Document& parent_doc) = 0;
    // };
    // using MultiVectorGuidancePtr = std::shared_ptr<MultiVectorGuidance>

    using MultiVectorGuidance = std::function<std::vector<Document>(const Document& parent_doc)>;

    struct MultiVectorRetrieverOptions {
        // metadata key for parent doc id
        std::string parent_doc_id_key = "parent_doc_id";

        // options for ChunkedMultiVectorRetriever


    };

    class MultiVectorRetriever: public IStatefulRetriever<TextQuery> {
        /**
         * document store for original documents
         */
        DocStorePtr doc_store_;

        /**
         * vector store for guidance docs, e.g. summary, chunked parts, hypothetical quries, etc. VectorStore should have document schema configured, containing the matching `parent_doc_id_key`.
         */
        VectorStorePtr vector_store_;

        /**
         * A chain to generate guidance docs
         */
        MultiVectorGuidance guidance_;

        MultiVectorRetrieverOptions options_;

    public:
        MultiVectorRetriever(DocStorePtr doc_store, VectorStorePtr vector_store, MultiVectorGuidance guidance_chain,
            MultiVectorRetrieverOptions options)
            : doc_store_(std::move(doc_store)),
              vector_store_(std::move(vector_store)),
              guidance_(std::move(guidance_chain)),
              options_(std::move(options)) {
        }


        ResultIteratorPtr<Document> Retrieve(const TextQuery& query) override {
            SearchRequest search_request;
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            const auto sub_doc_itr = vector_store_->SearchDocuments(search_request);
            std::unordered_set<std::string> parent_ids;
            while (sub_doc_itr->HasNext()) {
                parent_ids.insert(sub_doc_itr->Next().id());
            }
            if (parent_ids.empty()) {
                // TODO print warning
            }
            return doc_store_->MultiGetDocuments({parent_ids.begin(), parent_ids.end()});
        }

        void Ingest(const ResultIteratorPtr<Document>& input) override {
            while (input->HasNext()) {
                auto& parent_doc = input->Next();
                doc_store_->AddDocument(parent_doc);
                auto sub_docs = std::invoke(guidance_, parent_doc);
                for (auto& sub_doc: sub_docs) {
                    auto* field = parent_doc.add_metadata();
                    field->set_name(options_.parent_doc_id_key);
                    field->set_string_value(parent_doc.id());
                }
                UpdateResult update_result;
                vector_store_->AddDocuments(sub_docs, update_result);
                assert_true(update_result.failed_documents.empty(), "all sub docs should be inserted successfully");
            }
        }
    };
}

#endif //MULTIVECTORRETRIEVER_HPP
