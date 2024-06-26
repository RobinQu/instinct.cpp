//
// Created by RobinQu on 2024/3/12.
//

#ifndef MULTIQUERYRETRIEVER_HPP
#define MULTIQUERYRETRIEVER_HPP

#include <unordered_set>


#include <instinct/retrieval/BaseRetriever.hpp>
#include <instinct/RetrievalGlobals.hpp>
#include <instinct/chain/llm_chain.hpp>
#include <instinct/prompt/plain_prompt_template.hpp>
#include <instinct/tools/document_utils.hpp>



namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    class MultiQueryRetriever final: public BaseRetriever {
        RetrieverPtr base_retriever_;
        MultilineChainPtr query_chain_{};

    public:
        MultiQueryRetriever(RetrieverPtr base_retriever, MultilineChainPtr query_chain)
            : base_retriever_(std::move(base_retriever)),
              query_chain_(std::move(query_chain)) {
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest& search_request) const override {
            const auto queries = query_chain_->Invoke(search_request.query());
            assert_true(queries.lines_size() > 1, "should have multiple generated queries.");
            LOG_DEBUG("orignal query: {}, rewrite queries: {}", search_request.query(), queries.ShortDebugString());
            return rpp::source::from_iterable(queries.lines())
                | rpp::operators::flat_map([&, search_request](const std::string& q)   {
                    SearchRequest copied_request = search_request;
                    copied_request.set_query(q);
                    return base_retriever_->Retrieve(copied_request);
                })
                // see following specializations for std::hash and std::equal_to
                // modules/instinct-core/src/tools/DocumentUtils.hpp
                | rpp::operators::distinct();
        }
    };

    static RetrieverPtr CreateMultiQueryRetriever(
            const RetrieverPtr& base_retriever,
            const ChatModelPtr & llm,
            const PromptTemplatePtr& prompt_template = nullptr
    ) {
        auto query_chain = CreateMultilineChain(
                llm,
                prompt_template ? prompt_template : CreatePlainPromptTemplate(R"(You are an AI language model assistant. Your task is
    to generate 3 different versions of the given user
    question to retrieve relevant documents from a vector  database.
    By generating multiple perspectives on the user question,
    your goal is to help the user overcome some of the limitations
    of distance-based similarity search. Provide these alternative
    questions separated by newlines. Original question: {question})")
        );
        return std::make_shared<MultiQueryRetriever>(base_retriever, query_chain);
    }

}

#endif //MULTIQUERYRETRIEVER_HPP
