//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include "RetrievalGlobals.hpp"
#include "chain/BaseChain.hpp"
#include "chain/LLMChain.hpp"
#include "retrieval/IRetriever.hpp"
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct RAGChainOptions: ChainOptions {
        std::string context_output_key = "context";
        std::string condense_question_key = "standalone_question";
        int top_k = 10;
    };

    template<typename Result>
    class RAGChain final: public BaseChain<Result, RAGChainOptions> {
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
        ChainPtr<std::string> question_chain_;

        /**
         * to answer final question with context. this chain doesn't need memory
         */
        ChainPtr<Result> answer_chain_;

    public:
        RAGChain(
            ChatMemoryPtr chat_memory,
            RetrieverPtr retriever,
            ChainPtr<std::string> question_chain,
            ChainPtr<Result> answer_chain,
            const RAGChainOptions& options = {}
            )
            :   BaseChain<Result, RAGChainOptions>(options),
                chat_memory_(std::move(chat_memory)),
                retriever_(std::move(retriever)),
                question_chain_(std::move(question_chain)),
                answer_chain_(std::move(answer_chain)) {
            
        }

        void EnhanceContext(const ContextMutataorPtr& ctx_builder) override {
            if (chat_memory_) {
                // add chat history to context
                chat_memory_->EnhanceContext(ctx_builder);
            }
            const auto question_string = question_chain_->Invoke(ctx_builder->Build());
            const auto doc_itr = retriever_->Retrieve({.text = question_string, .top_k = this->GetOptions().top_k});
            std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
            ctx_builder->Put(this->GetOptions().context_output_key, context_string);
            ctx_builder->Put(this->GetOptions().condense_question_key, question_string);
            ctx_builder->Commit();
        }

        Result Invoke(const ContextPtr& input) override {
            const auto ctx_builder = ContextMutataor::Create(input);
            EnhanceContext(ctx_builder);
            Result result = answer_chain_->Invoke(ctx_builder->Build());
            if (chat_memory_) {
                chat_memory_->SaveMemory(ctx_builder->Build());
            }
            return result;
        }

    };

    template<typename T>
    using RAGChainPtr = ChainPtr<T, RAGChainOptions>;

}


#endif //RAGCHAIN_HPP
