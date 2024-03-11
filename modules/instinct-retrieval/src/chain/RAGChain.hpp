//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include "RetrievalGlobals.hpp"
#include "chain/ChainContextBuilder.hpp"
#include "chain/IChain.hpp"
#include "chain/LLMChain.hpp"
#include "retrieval/IRetriever.hpp"
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    template<typename Result>
    class RAGChain: public IChain<Result> {
        /**
         * to fetch external knowledge (or documents)
         */
        RetrieverPtr retriever_;

        /**
         *  to augmented question prompt
         */
        LLMChainPtr<std::string> question_chain_;

        /**
         * to answer final question with context
         */
        LLMChainPtr<Result> answer_chain_;

    public:
        RAGChain(RetrieverPtr retriever, LLMChainPtr<std::string> question_chain, LLMChainPtr<Result> answer_chain)
            : retriever_(std::move(retriever)),
              question_chain_(std::move(question_chain)),
              answer_chain_(std::move(answer_chain)) {
        }

        Result Invoke(const LLMChainContext& input) override {
            const auto question_string = question_chain_->Invoke(input);
            const auto doc_itr = retriever_->Retrieve(question_string);
            std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
            const auto ctx_builder = ChainContextBuilder::Create(input);
            ctx_builder->Put("context", context_string);
            return answer_chain_->Invoke(ctx_builder->Build());
        }

        ResultIteratorPtr<Result> Batch(const std::vector<LLMChainContext>& input) override {
            const auto batched_question_string = question_chain_->Batch(input);
            // TODO make retriever batchable
            int i=0;
            std::vector<LLMChainContext> augumented_contexts;
            augumented_contexts.reserve(input.size());
            while (batched_question_string->HasNext() && i < input.size()) {
                const auto doc_itr = retriever_->Retrieve(batched_question_string->Next());
                std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
                const auto ctx_builder = ChainContextBuilder::Create(input[i++]);
                ctx_builder->Put("context", context_string);
                augumented_contexts.push_back(ctx_builder->Build());
            }
            return answer_chain_->Batch(augumented_contexts);
        }

        ResultIteratorPtr<Result> Stream(const LLMChainContext& input) override {
            const auto question_string = question_chain_->Invoke(input);
            const auto doc_itr = retriever_->Retrieve(question_string);
            std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
            const auto ctx_builder = ChainContextBuilder::Create(input);
            ctx_builder->Put("context", context_string);
            return answer_chain_->Stream(ctx_builder->Build());
        }
    };

}


#endif //RAGCHAIN_HPP
