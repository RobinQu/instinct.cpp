//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include "RetrievalGlobals.hpp"
#include "chain/ChainContextBuilder.hpp"
#include "chain/BaseChain.hpp"
#include "chain/LLMChain.hpp"
#include "retrieval/IRetriever.hpp"
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct RAGChainOptions: ChainOptions {
        std::string context_output_key = "context";
        std::string condense_question_key = "standalone_question";
    };

    template<typename Result>
    class RAGChain: public BaseChain<Result, RAGChainOptions> {
        // RAGChainOptions rag_chain_options_;
        /**
         * converation memory
         */
        ChatMemoryPtr chat_memory_;

        /**
         * to fetch external knowledge (or documents)
         */
        RetrieverPtr retriever_;

        /**
         *  to augmented question prompt. this chain doesn't need memory
         */
        LLMChainPtr<std::string> question_chain_;

        /**
         * to answer final question with context. this chain doesn't need memory
         */
        LLMChainPtr<Result> answer_chain_;

    public:
        RAGChain(
            ChatMemoryPtr chat_memory,
            RetrieverPtr retriever,
            LLMChainPtr<std::string> question_chain,
            LLMChainPtr<Result> answer_chain,
            ChainOptions options
            )
            :   BaseChain<Result>(std::move(options)),
                chat_memory_(std::move(chat_memory)),
                retriever_(std::move(retriever)),
                question_chain_(std::move(question_chain)),
                answer_chain_(std::move(answer_chain)) {
        }

        void EnhanceContext(const ChainContextBuilderPtr& ctx_builder) override {
            if (chat_memory_) {
                // add chat history to context
                chat_memory_->EnhanceContext(ctx_builder);
            }
            const auto question_string = question_chain_->Invoke(ctx_builder->Build());
            const auto doc_itr = retriever_->Retrieve(question_string);
            std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
            ctx_builder->Put(this->GetOptions().context_output_key, context_string);
            ctx_builder->Put(this->GetOptions().condense_question_key, question_string);
        }

        Result Invoke(const LLMChainContext& input) override {
            const auto ctx_builder = ChainContextBuilder::Create(input);
            EnhanceContext(ctx_builder);
            Result result = answer_chain_->Invoke(ctx_builder->Build());
            if (chat_memory_) {
                chat_memory_->SaveMemory(ctx_builder->Build());
            }
            return result;
        }
        //
        // ResultIteratorPtr<Result> Batch(const std::vector<LLMChainContext>& input) override {
        //     const auto batched_question_string = question_chain_->Batch(input);
        //     // TODO make retriever batchable
        //     int i=0;
        //     std::vector<LLMChainContext> augumented_contexts;
        //     augumented_contexts.reserve(input.size());
        //     while (batched_question_string->HasNext() && i < input.size()) {
        //         const auto doc_itr = retriever_->Retrieve(batched_question_string->Next());
        //         std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
        //         const auto ctx_builder = ChainContextBuilder::Create(input[i++]);
        //         ctx_builder->Put("context", context_string);
        //         augumented_contexts.push_back(ctx_builder->Build());
        //     }
        //     return answer_chain_->Batch(augumented_contexts);
        // }
        //
        // ResultIteratorPtr<Result> Stream(const LLMChainContext& input) override {
        //     const auto question_string = question_chain_->Invoke(input);
        //     const auto doc_itr = retriever_->Retrieve(question_string);
        //     std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
        //     const auto ctx_builder = ChainContextBuilder::Create(input);
        //     ctx_builder->Put("context", context_string);
        //     return answer_chain_->Stream(ctx_builder->Build());
        // }
    };

}


#endif //RAGCHAIN_HPP
