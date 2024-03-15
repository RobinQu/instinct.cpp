//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include "RetrievalGlobals.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename Query>
    class IRetriever {
    public:
        IRetriever()=default;
        virtual ~IRetriever() = default;
        IRetriever(const IRetriever&)=delete;
        IRetriever(IRetriever&&)=delete;

        virtual AsyncIterator<Document> Retrieve(const Query& query) = 0;
    };


    struct TextQuery {
        std::string text;
        int top_k = 10;
    };
    class ITextRetreiver: public IRetriever<TextQuery> {};


    struct GuidedQuery {
        AsyncIterator<Document> guidance_docs_iterator;
        TextQuery raw_query;
    };
    class IGuidedRetreiver : public IRetriever<GuidedQuery> {};

    using RetrieverPtr = std::shared_ptr<ITextRetreiver>;
    using GuidedRetreiverPtr = std::shared_ptr<IGuidedRetreiver>;

}

#endif //DOCUMENTRETRIEVAL_HPP
