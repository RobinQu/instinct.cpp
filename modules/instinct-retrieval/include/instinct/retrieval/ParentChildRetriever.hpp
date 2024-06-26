//
// Created by RobinQu on 2024/6/18.
//

#ifndef PARENTCHILDRETRIEVER_HPP
#define PARENTCHILDRETRIEVER_HPP


#include <instinct/retrieval/BaseRetriever.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    class ParentChildRetriever: public BaseRetriever {
        DocStorePtr parent_doc_store_;
        StatefulRetrieverPtr child_retriever_;
    public:
        ParentChildRetriever(DocStorePtr parent_doc_store, StatefulRetrieverPtr child_retriever)
            : parent_doc_store_(std::move(parent_doc_store)),
              child_retriever_(std::move(child_retriever)) {
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest &search_request) const override {
            return child_retriever_->Retrieve(search_request)
                | rpp::operators::reduce(std::unordered_set<std::string> {}, [&](std::unordered_set<std::string>&& seed, const Document& doc) {
                        // backtrace id of parent doc
                        const auto parent_doc_id = DocumentUtils::GetStringValueMetadataField(doc, METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                        assert_true(parent_doc_id && StringUtils::IsNotBlankString(parent_doc_id.value()), "should have found parent_doc_id in parent doc's metadata");
                        LOG_DEBUG("guidance doc found, id={}, parent_doc_id={}", doc.id(), parent_doc_id.value());
                        seed.insert(parent_doc_id.value());
                        return std::move(seed);
                    })
                    | rpp::operators::flat_map([&](const std::unordered_set<std::string>& parent_ids) {
                        if (parent_ids.empty()) {
                            LOG_WARN("Retrieved empty result set with query ");
                        } else {
                            LOG_DEBUG("backtrace parent doc ids: {}", parent_ids);
                        }
                        return parent_doc_store_->MultiGetDocuments({parent_ids.begin(), parent_ids.end()});
                    });
        }
    };

    static RetrieverPtr CreateParentChildRetriever(
        const DocStorePtr& parent_doc_store,
        const StatefulRetrieverPtr& child_retriever
    ) {
        return std::make_shared<ParentChildRetriever>(parent_doc_store, child_retriever);
    }


    class StatefulParentChildRetriever: public BaseStatefulRetriever {
        DocStorePtr parent_doc_store_;
        StatefulRetrieverPtr child_retriever_;
    public:
        StatefulParentChildRetriever(DocStorePtr parent_doc_store, StatefulRetrieverPtr child_retriever)
            : parent_doc_store_(std::move(parent_doc_store)),
              child_retriever_(std::move(child_retriever)) {
        }

        void Remove(const SearchQuery &metadata_query) override {
            UpdateResult update_result;
            parent_doc_store_->DeleteDocuments(metadata_query, update_result);
            assert_true(update_result.failed_documents_size()==0);
            child_retriever_->Remove(metadata_query);
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest &search_request) const override {
            return child_retriever_->Retrieve(search_request)
            | rpp::operators::reduce(std::unordered_set<std::string> {}, [&](std::unordered_set<std::string>&& seed, const Document& doc) {
                    // backtrace id of parent doc
                    const auto parent_doc_id = DocumentUtils::GetStringValueMetadataField(doc, METADATA_SCHEMA_PARENT_DOC_ID_KEY);
                    assert_true(parent_doc_id && StringUtils::IsNotBlankString(parent_doc_id.value()), "should have found parent_doc_id in parent doc's metadata");
                    LOG_DEBUG("guidance doc found, id={}, parent_doc_id={}", doc.id(), parent_doc_id.value());
                    seed.insert(parent_doc_id.value());
                    return std::move(seed);
                })
                | rpp::operators::flat_map([&](const std::unordered_set<std::string>& parent_ids) {
                    if (parent_ids.empty()) {
                        LOG_WARN("Retrieved empty result set with query ");
                    } else {
                        LOG_DEBUG("backtrace parent doc ids: {}", parent_ids);
                    }
                    return parent_doc_store_->MultiGetDocuments({parent_ids.begin(), parent_ids.end()});
                });
        }

        virtual std::vector<Document> GenerateChildDocuments(const Document& parent_doc) = 0;

        void Ingest(const AsyncIterator<Document> &input) override {
            static int BATCH_SIZE = 50;
            input
            | rpp::operators::as_blocking()
            | rpp::operators::buffer(BATCH_SIZE)
            | rpp::operators::subscribe([&](const std::vector<Document>& buf) {
                std::vector<Document> batch = buf;
                // insert all parent docs
                UpdateResult updateResult;
                parent_doc_store_->AddDocuments(batch, updateResult);
                // insert all child docs
                for(const auto& parent_doc: batch) {
                    auto sub_docs = GenerateChildDocuments(parent_doc);
                    LOG_DEBUG("{} child doc(s) generated for parent doc with id {}", sub_docs.size(), parent_doc.id());
                    child_retriever_->Ingest(rpp::source::from_iterable(sub_docs));
                    UpdateResult update_result;
                }
            });
        }

        DocStorePtr GetDocStore() override {
            return parent_doc_store_;
        }
    };
}

#endif //PARENTCHILDRETRIEVER_HPP
