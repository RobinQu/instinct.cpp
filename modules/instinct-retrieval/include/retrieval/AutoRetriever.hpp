//
// Created by RobinQu on 2024/3/12.
//

#ifndef AUTORETRIEVER_HPP
#define AUTORETRIEVER_HPP

#include "IRetriever.hpp"
#include "LLMGlobals.hpp"
#include "RetrievalGlobals.hpp"
#include "chain/BaseChain.hpp"
#include "chain/LLMChain.hpp"
#include "llm/BaseLLM.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "output_parser/MetadataQueryOutputParser.hpp"
#include "store/IVectorStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    using MetadataFilterChainPtr = ChainPtr<MetadataQuery>;

    class AutoRetriever final: public ITextRetriever {
        /**
         * retriever to do actual search
         */
        VectorStorePtr vector_store_;

        /**
         * LLMChain to genearte MetadataQuery for given text query
         */
        MetadataFilterChainPtr metadata_filter_chain_ {};
    public:

        AutoRetriever(
            VectorStorePtr vector_store,
            const LLMPtr& llm,
            const PromptTemplatePtr& prompt_template
            )
            : vector_store_(std::move(vector_store)) {
            auto output_parser = std::make_shared<MetadataQueryOutputParser>(vector_store_->GetMetadataSchema());
            metadata_filter_chain_ = std::make_shared<LLMChain<MetadataQuery>>(
                    llm,
                    prompt_template,
                    output_parser
                );
        }

        AsyncIterator<Document> Retrieve(const TextQuery& query) const override {
            const auto context_builder = ContextMutataor::Create();
            context_builder->Put(metadata_filter_chain_->GetInputKeys()[0], query.text);
            auto filter = metadata_filter_chain_->Invoke(context_builder->Build());
            SearchRequest search_request;
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            search_request.mutable_metadata_filter()->CopyFrom(filter);
            return vector_store_->SearchDocuments(search_request);
        }
    };
}

#endif //AUTORETRIEVER_HPP
