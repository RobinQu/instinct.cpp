//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include "RetrievalGlobals.hpp"
#include "functional/ReactiveFunctions.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    template<typename Query>
    class IRetriever {
    public:
        IRetriever()=default;
        virtual ~IRetriever() = default;
        IRetriever(const IRetriever&)=delete;
        IRetriever(IRetriever&&)=delete;

        [[nodiscard]] virtual AsyncIterator<Document> Retrieve(const Query& query) const = 0;
    };


    struct TextQuery {
        std::string text;
        int top_k = 10;
    };
//    class ITextRetriever: public IRetriever<TextQuery> {
//    public:
//        virtual AsyncIterator<Document> Retrieve(const TextQuery &query) = 0;
//    };
    using ITextRetriever = IRetriever<TextQuery>;


    struct GuidedQuery {
        AsyncIterator<Document> guidance_docs_iterator;
        TextQuery raw_query;
    };
    class IGuidedRetriever : public IRetriever<GuidedQuery> {};
    using GuidedRetreiverPtr = std::shared_ptr<IGuidedRetriever>;


    class IStatefulRetriever {
    public:
        virtual void Ingest(const AsyncIterator<Document>& input) = 0;
    };

}

#endif //DOCUMENTRETRIEVAL_HPP
