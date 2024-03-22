//
// Created by RobinQu on 2024/3/9.
//

#ifndef RAGCHAIN_HPP
#define RAGCHAIN_HPP

#include <utility>

#include "RetrievalGlobals.hpp"
#include "chain/MessageChain.hpp"
#include "chain/LLMChain.hpp"
#include "retrieval/BaseRetriever.hpp"
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    template<typename Result>
    class RAGChain;
    template<typename T>
    using RAGChainPtr = RAGChain<T>;

    template<typename Result>
    class RAGChain final : public MessageChain<PromptValue, Result> {
        /**
         * conversation memory
         */
        ChatMemoryPtr chat_memory_;

        /**
         * to fetch external knowledge (or documents)
         */
        RetrieverPtr retriever_;

        /**
         *  to augmented question prompt. this chain doesn't need memory
         */
        TextChainPtr question_chain_;

        /**
         * to answer final question with context. this chain doesn't need memory
         */
        MessageChainPtr<PromptValue, Result> answer_chain_;

        /**
         * RAG related options
         */
        RAGChainOptions options_;

    public:
        RAGChain(
                ChatMemoryPtr chat_memory,
                RetrieverPtr retriever,
                TextChainPtr question_chain,
                MessageChainPtr<PromptValue, Result> answer_chain,
                RAGChainOptions options = {}
        ) : MessageChain<PromptValue, Result>(
                question_chain->GetInputParser(),
                answer_chain->GetOutputParser()
        ),
            chat_memory_(std::move(chat_memory)),
            retriever_(std::move(retriever)),
            question_chain_(question_chain),
            answer_chain_(answer_chain),
            options_(std::move(options)) {}

        StepFunctionPtr GetStepFunction() override {
            return chat_memory_->AsLoadMemoryFunction()
                | question_chain_->GetStepFunction()
                | retriever_->AsContextRetrieverFunction(options_)
                | answer_chain_->GetStepFunction()
                | chat_memory_->AsSaveMemoryFunction();
        }

        static RAGChainPtr<Result> CreateRAGChain(
                RetrieverPtr retriever,
                TextChainPtr question_chain,
                MessageChainPtr<PromptValue, Result> answer_chain,
                ChatMemoryPtr chat_memory = nullptr,
                const RAGChainOptions &options = {}
        ) {
            return std::make_shared<RAGChain<std::string>>(
                    chat_memory,
                    retriever,
                    question_chain,
                    answer_chain
            );
        }

    };





}


#endif //RAGCHAIN_HPP
