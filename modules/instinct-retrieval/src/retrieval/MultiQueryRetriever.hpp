//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIQUERYRETRIEVER_HPP
#define MULTIQUERYRETRIEVER_HPP


#include "IRetriever.hpp"
#include "RetrievalGlobals.hpp"
#include "chain/BaseChain.hpp"
#include "chain/LLMChain.hpp"
#include "llm/BaseLLM.hpp"
#include "output_parser/MultiLineTextOutputParser.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    class MultiqueryRetriever: public ITextRetreiver {
        RetrieverPtr base_retriever_;
        MultilineTextChainPtr query_chain_{};

    public:

        static RetrieverPtr FromLanguageModel(
            const RetrieverPtr& base_retriever,
            const LLMPtr& llm,
            const PromptTemplatePtr& prompt_template
            ) {
            const auto output_parse = std::make_shared<MultiLineTextOutputParser>();
            auto query_chain = std::make_shared<MultilineTextLLMChain>(
                llm,
                prompt_template,
                output_parse,
                nullptr
                );
            return std::make_shared<MultiqueryRetriever>(base_retriever, query_chain);
        }

        MultiqueryRetriever(RetrieverPtr base_retriever, MultilineTextChainPtr query_chain)
            : base_retriever_(std::move(base_retriever)),
              query_chain_(std::move(query_chain)) {
        }

        AsyncIterator<Document> Retrieve(const TextQuery& query) override {
            const auto context_builder = ContextMutataor::Create();
            context_builder->Put(query_chain_->GetInputKeys()[0], query.text);
            const auto queries = query_chain_->Invoke(context_builder->Build());
            assert_true(queries.size()>1, "should have multipe generated queries.");

            std::unordered_set<std::string> ids;
            return rpp::source::from_iterable(queries)
                | rpp::operators::as_blocking()
                | rpp::operators::flat_map([&](const std::string& q) {
                    TextQuery text_query = query;
                    text_query.text = q;
                    return base_retriever_->Retrieve(text_query);
                    //return rpp::source::empty<Document>();
                })
                | rpp::operators::filter([&](const Document& doc) {
                    if (ids.contains(doc.id())) {
                        return false;
                    }
                    ids.insert(doc.id());
                    return true;
                });
        }
    };
}

#endif //MULTIQUERYRETRIEVER_HPP
