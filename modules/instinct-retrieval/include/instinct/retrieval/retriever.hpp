//
// Created by RobinQu on 2024/2/28.
//

#ifndef DOCUMENTRETRIEVAL_HPP
#define DOCUMENTRETRIEVAL_HPP

#include <instinct/retrieval_global.hpp>
#include <instinct/functional/reactive_functions.hpp>

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_CORE_NS;

    struct TextQuery {
        std::string text;
        int top_k = 10;
    };

    class IRetriever {
    public:
        IRetriever()=default;
        virtual ~IRetriever() = default;
        IRetriever(const IRetriever&)=delete;
        IRetriever(IRetriever&&)=delete;

        /**
         * Search with text query
         * @param query
         * @return
         */
        [[nodiscard]] virtual AsyncIterator<Document> Retrieve(const TextQuery& query) const = 0;


    };


    class IStatefulRetriever {
    public:
        virtual ~IStatefulRetriever()=default;
        virtual void Ingest(const AsyncIterator<Document>& input) = 0;
        virtual void Remove(const SearchQuery& metadata_query) = 0;
    };

}

#endif //DOCUMENTRETRIEVAL_HPP
