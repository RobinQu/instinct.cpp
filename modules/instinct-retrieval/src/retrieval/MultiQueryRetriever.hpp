//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIQUERYRETRIEVER_HPP
#define MULTIQUERYRETRIEVER_HPP


#include "BaseRetriever.h"
#include "RetrievalGlobals.hpp"
#include "chain/BaseChain.hpp"
#include "chain/LLMChain.hpp"
#include "llm/BaseLLM.hpp"
#include "output_parser/MultiLineTextOutputParser.hpp"
#include "prompt/PlainPromptTemplate.hpp"
#include <unordered_set>
#include "retrieval/DocumentUtils.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    class MultiQueryRetriever: public BaseRetriever {
        RetrieverPtr base_retriever_;
        MultilineTextChainPtr query_chain_{};

    public:
        MultiQueryRetriever(RetrieverPtr base_retriever, MultilineTextChainPtr query_chain)
            : base_retriever_(std::move(base_retriever)),
              query_chain_(std::move(query_chain)) {
        }

        AsyncIterator<Document> Retrieve(const TextQuery& query) override {
            const auto context_builder = ContextMutataor::Create();
            context_builder->Put(query_chain_->GetInputKeys()[0], query.text);
            const auto queries = query_chain_->Invoke(context_builder->Build());
            assert_true(queries.size()>1, "should have multiple generated queries.");

            return rpp::source::from_iterable(queries)
                | rpp::operators::flat_map([&](const std::string& q) {
                    TextQuery text_query = query;
                    text_query.text = q;
                    return base_retriever_->Retrieve(text_query);
                })
                // see following specializations for std::hash and std::equal_to
                // modules/instinct-retrieval/src/retrieval/DocumentUtils.hpp:14
                // modules/instinct-retrieval/src/retrieval/DocumentUtils.hpp:21
                | rpp::operators::distinct();
        }
    };

    static RetrieverPtr CreateMultiQueryRetriever(
            const RetrieverPtr& base_retriever,
            const ChatModelPtr & llm,
            const PromptTemplatePtr& prompt_template = nullptr
    ) {
        const auto output_parse = std::make_shared<MultiLineTextOutputParser>();
        auto query_chain = std::make_shared<MultilineTextLLMChain>(
                llm,
                prompt_template ? prompt_template : PlainPromptTemplate::CreateWithTemplate(R"(You are an AI language model assistant. Your task is
    to generate 3 different versions of the given user
    question to retrieve relevant documents from a vector  database.
    By generating multiple perspectives on the user question,
    your goal is to help the user overcome some of the limitations
    of distance-based similarity search. Provide these alternative
    questions separated by newlines. Original question: {question})"),
                output_parse,
                nullptr
        );
        return std::make_shared<MultiQueryRetriever>(base_retriever, query_chain);
    }

}

#endif //MULTIQUERYRETRIEVER_HPP
