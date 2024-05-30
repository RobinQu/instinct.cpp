//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIVECTORRETRIEVER_HPP
#define MULTIVECTORRETRIEVER_HPP


#include "BaseRetriever.hpp"
#include "chain/LLMChain.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "store/IVectorStore.hpp"
#include <fmt/ranges.h>


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    using MultiVectorGuidance = std::function<std::vector<Document>(const Document& parent_doc)>;

    struct MultiVectorRetrieverOptions {
        // metadata key for parent doc id
        // std::string parent_doc_id_key = METADATA_SCHEMA_PARENT_DOC_ID_KEY;
    };

    class MultiVectorRetriever: public BaseStatefulRetriever {
        /**
         * document store for original documents
         */
        DocStorePtr doc_store_;

        /**
         * vector store for guidance docs, e.g. summary, chunked parts, hypothetical queries, etc. VectorStore should have document schema configured, containing the matching `parent_doc_id_key`.
         */
        VectorStorePtr vector_store_;

        /**
         * A chain to generate guidance docs
         */
        MultiVectorGuidance guidance_;

        /**
         * options for this retriever
         */
        MultiVectorRetrieverOptions options_;

    public:
        MultiVectorRetriever(
            DocStorePtr doc_store,
            VectorStorePtr vector_store,
            MultiVectorGuidance guidance_chain,
            const MultiVectorRetrieverOptions& options)
            : doc_store_(std::move(doc_store)),
              vector_store_(std::move(vector_store)),
              guidance_(std::move(guidance_chain)),
              options_(options) {
            assert_true(doc_store_, "should have doc store");
            assert_true(vector_store_, "should have doc store");
            assert_true(guidance_, "should have guidance");
        }

        DocStorePtr GetDocStore() override {
            return doc_store_;
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const TextQuery& query) const override {
            SearchRequest search_request;
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            // result can be less than `top_k`.
            return vector_store_->SearchDocuments(search_request)
                | rpp::operators::reduce(std::unordered_set<std::string> {}, [&](std::unordered_set<std::string>&& seed, const Document& doc) {
                    // backtrace id of parent doc
                    for(const auto& metadata_field: doc.metadata()) {
                        if (metadata_field.name() == METADATA_SCHEMA_PARENT_DOC_ID_KEY) {
                            LOG_DEBUG("guidance doc found, id={}, parent_doc_id={}", doc.id(), metadata_field.string_value());
                            seed.insert(metadata_field.string_value());
                        }
                    }
                    return std::move(seed);
                })
                | rpp::operators::flat_map([&](const std::unordered_set<std::string>& parent_ids) {
                    if (parent_ids.empty()) {
                        LOG_WARN("Retrieved empty result set with query ");
                    } else {
                        LOG_DEBUG("backtrace parent doc ids: {}", parent_ids);
                    }
                    return doc_store_->MultiGetDocuments({parent_ids.begin(), parent_ids.end()});
                });
        }

        void Ingest(const AsyncIterator<Document>& input) override {
            Futures<void> tasks;
            input
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([&](const Document& parent_doc) {
                auto f = IO_WORKER_POOL.submit_task([&,parent_doc]() {
                    Document copied_doc = parent_doc;
                    doc_store_->AddDocument(copied_doc);
                    auto sub_docs = std::invoke(guidance_, copied_doc);
                        LOG_DEBUG("{} guidance doc(s) generated for parent doc with id {}", sub_docs.size(), copied_doc.id());
                    UpdateResult update_result;
                    vector_store_->AddDocuments(sub_docs, update_result);
                    assert_true(update_result.failed_documents_size()==0, "all sub docs should be inserted successfully");
                });
                tasks.push_back(std::move(f));
            });

            tasks.wait();
        }

        void Remove(const SearchQuery &metadata_query) override {
            UpdateResult update_result;
            doc_store_->DeleteDocuments(metadata_query, update_result);
            // if (update_result.failed_documents_size() == 0) {
            // }
            vector_store_->DeleteDocuments(metadata_query, update_result);
            // TODO handle exceptions
        }
    };

    static StatefulRetrieverPtr CreateSummaryGuidedRetriever(
        const ChatModelPtr& llm,
        const DocStorePtr& doc_store,
        const VectorStorePtr& vector_store,
        const PromptTemplatePtr& prompt_template = nullptr,
        const MultiVectorRetrieverOptions& options = {}
        ) {
        const TextChainPtr summary_chain = CreateTextChain(
            llm,
            // prompt is copied from langchain doc, which may not be the best choice
            // https://python.langchain.com/docs/modules/data_connection/retrievers/multi_vector#summary
            prompt_template == nullptr ? CreatePlainPromptTemplate("Summarize the following document:\n\n{question}", {.input_keys = {"question"}}) : prompt_template
            );
        MultiVectorGuidance guidance = [&, summary_chain](const Document& doc) {
            assert_true(!StringUtils::IsBlankString(doc.id()), "should have valid doc id");
            auto generation = summary_chain->Invoke(
                    doc.text()
            );
            LOG_DEBUG("Genearted summary: {}", generation);
            Document summary_doc;
            summary_doc.set_text(generation);
            summary_doc.set_id(StringUtils::GenerateUUIDString());
            DocumentUtils::AddPresetMetadataFileds(
                summary_doc,
                doc.id()
            );
            return std::vector {std::move(summary_doc)};
        };

        return std::make_shared<MultiVectorRetriever>(doc_store, vector_store, guidance, options);
    }

    [[maybe_unused]] static StatefulRetrieverPtr CreateHypotheticalQueriesGuidedRetriever(
        const ChatModelPtr& llm,
        const DocStorePtr& doc_store,
        const VectorStorePtr& vector_store,
        const PromptTemplatePtr& prompt_template = nullptr,
        const MultiVectorRetrieverOptions& options = {}
        ) {
        auto query_chain = CreateMultilineChain(
            llm,
            // prompt is copied from langchain doc, which may not be the best choice
            // https://python.langchain.com/docs/modules/data_connection/retrievers/multi_vector#hypothetical-queries
            prompt_template == nullptr ? CreatePlainPromptTemplate(
                    "Generate a list of exactly 3 hypothetical questions that the below document could be used to answer:\n\n{question}",
                    {.input_keys = {"question"}}) : prompt_template
            );

        MultiVectorGuidance guidance = [&, query_chain](const Document& doc) {
            assert_true(!StringUtils::IsBlankString(doc.id()), "should have valid doc id");
            const auto result = query_chain->Invoke(doc.text());
            std::vector<Document> final_queries;
            LOG_DEBUG("Genearted queries: {}", result.ShortDebugString());
            for(const auto& query: result.lines()) {
                Document query_doc;
                query_doc.set_id(StringUtils::GenerateUUIDString());
                query_doc.set_text(query);
                DocumentUtils::AddPresetMetadataFileds(query_doc, doc.id());
                final_queries.push_back(query_doc);
            }
            return final_queries;
        };
        return std::make_shared<MultiVectorRetriever>(doc_store, vector_store, guidance,  options);
    }


}

#endif //MULTIVECTORRETRIEVER_HPP
