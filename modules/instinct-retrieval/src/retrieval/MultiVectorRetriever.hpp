//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIVECTORRETRIEVER_HPP
#define MULTIVECTORRETRIEVER_HPP


#include "IStatefulRetriever.hpp"
#include "chain/LLMChain.hpp"
#include "output_parser/StringOutputParser.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include "store/IVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    using MultiVectorGuidance = std::function<std::vector<Document>(const Document& parent_doc)>;

    struct MultiVectorRetrieverOptions {
        // metadata key for parent doc id
        std::string parent_doc_id_key = "parent_doc_id";
    };

    class MultiVectorRetriever: public IStatefulRetriever<TextQuery> {
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
        MultiVectorRetriever(DocStorePtr doc_store, VectorStorePtr vector_store, MultiVectorGuidance guidance_chain,
            MultiVectorRetrieverOptions options)
            : doc_store_(std::move(doc_store)),
              vector_store_(std::move(vector_store)),
              guidance_(std::move(guidance_chain)),
              options_(std::move(options)) {
            assert_true(!!doc_store_, "should have doc store");
            assert_true(!!vector_store_, "should have doc store");
            // assert_true(typeid(*doc_store_) != typeid(*vector_store_), "cannot use a VectorStore both for doc and embedding.");
        }


        AsyncIterator<Document> Retrieve(const TextQuery& query) override {
            SearchRequest search_request;
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            return vector_store_->SearchDocuments(search_request)
                | rpp::operators::reduce(std::unordered_set<std::string> {}, [&](std::unordered_set<std::string>&& seed, const Document& doc) {
                    // backtrace id of parent doc
                    for(const auto& metadata_field: doc.metadata()) {
                        if (metadata_field.name() == options_.parent_doc_id_key) {
                            seed.insert(metadata_field.string_value());
                        }
                    }
                    return std::move(seed);
                })
                | rpp::operators::flat_map([&](const std::unordered_set<std::string>& parent_ids) {
                    if (parent_ids.empty()) {
                        LOG_WARN("Retrieved empty result set with query ");
                    }
                    return doc_store_->MultiGetDocuments({parent_ids.begin(), parent_ids.end()});
                });
        }

        void Ingest(const AsyncIterator<Document>& input) override {
            std::vector<Document> guided_docs;
            auto parent_docs = CollectVector(input);
            for (auto& parent_doc: parent_docs) {
                // auto& parent_doc = input->Next();
                doc_store_->AddDocument(parent_doc);
                auto sub_docs = std::invoke(guidance_, parent_doc);
                for (auto& sub_doc: sub_docs) {
                    auto* field = sub_doc.add_metadata();
                    field->set_name(options_.parent_doc_id_key);
                    field->set_string_value(parent_doc.id());
                }
                guided_docs.insert(guided_docs.end(), sub_docs.begin(), sub_docs.end());
            }
            UpdateResult update_result;
            vector_store_->AddDocuments(guided_docs, update_result);
            assert_true(update_result.failed_documents_size()==0, "all sub docs should be inserted successfully");
        }
    };

    static StatefulRetrieverPtr CreateSummaryGuidedRetriever(
        const ChatModelPtr& llm,
        const DocStorePtr& doc_store,
        const VectorStorePtr& vector_store,
        const PromptTemplatePtr& prompt_template = nullptr,
        const MultiVectorRetrieverOptions& options = {}
        ) {
        TextOutputParserPtr output_parser = std::make_shared<StringOutputParser>();
        ChainOptions chain_options = {.input_keys = {"doc"}};
        const TextChainPtr summary_chain = std::make_shared<TextLLMChain>(
            llm,
            // prompt is copied from langchain doc, which may not be the best choice
            // https://python.langchain.com/docs/modules/data_connection/retrievers/multi_vector#summary
            prompt_template == nullptr ? PlainPromptTemplate::CreateWithTemplate("Summarize the following document:\n\n{doc}") : prompt_template,
            output_parser,
            nullptr,
            chain_options
            );
        MultiVectorGuidance guidance = [&, summary_chain](const Document& doc) {
            assert_true(!StringUtils::IsBlankString(doc.id()), "should have valid doc id");
            const auto context_builder = ContextMutataor::Create();
            context_builder->Put(summary_chain->GetInputKeys()[0], doc.text());
            Document summary_doc;
            const auto result = summary_chain->Invoke(context_builder->Build());
            summary_doc.set_text(result);
            summary_doc.set_id(u8_utils::uuid_v8());
            auto* parent_id_field = summary_doc.add_metadata();
            parent_id_field->set_name(options.parent_doc_id_key);
            parent_id_field->set_string_value(doc.id());
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
        auto output_parser = std::make_shared<MultiLineTextOutputParser>();

        ChainOptions chain_options = {.input_keys = {"doc"}};
        auto query_chain = std::make_shared<MultilineTextLLMChain>(
            llm,
            // prompt is copied from langchian doc, which may not be the best choice
            // https://python.langchain.com/docs/modules/data_connection/retrievers/multi_vector#hypothetical-queries
            prompt_template == nullptr ? PlainPromptTemplate::CreateWithTemplate("Generate a list of exactly 3 hypothetical questions that the below document could be used to answer:\n\n{doc}") : prompt_template,
            output_parser,
            nullptr,
            chain_options
            );

        MultiVectorGuidance guidance = [&, query_chain](const Document& doc) {
            assert_true(!StringUtils::IsBlankString(doc.id()), "should have valid doc id");
            const auto context_builder = ContextMutataor::Create();
            context_builder->Put(query_chain->GetInputKeys()[0], doc.text());
            auto result = query_chain->Invoke(context_builder->Build());

            auto queries_view = result | std::views::transform([&](const std::string& query) {
                Document query_doc;
                query_doc.set_id(u8_utils::uuid_v8());
                query_doc.set_text(query);
                auto* parent_id_field = query_doc.add_metadata();
                parent_id_field->set_name(options.parent_doc_id_key);
                parent_id_field->set_string_value(doc.id());
                return query_doc;
            });

            return std::vector(queries_view.begin(), queries_view.end());
        };
        return std::make_shared<MultiVectorRetriever>(doc_store, vector_store, guidance,  options);
    }


}

#endif //MULTIVECTORRETRIEVER_HPP
