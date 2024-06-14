//
// Created by RobinQu on 2024/3/17.
//

#ifndef INSTINCT_BASERETRIEVER_HPP
#define INSTINCT_BASERETRIEVER_HPP

#include "retrieval/IRetriever.hpp"
#include "tools/DocumentUtils.hpp"
#include "functional/StepFunctions.hpp"
#include "store/IDocStore.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct RetrieverFunctionOptions {
        int top_k = 10;
    };

    class BaseRetriever : public IRetriever {
    public:

        [[nodiscard]] AsyncIterator<Document> Retrieve(const TextQuery &query) const override {
            SearchRequest search_request;
            search_request.set_query(query.text);
            search_request.set_top_k(query.top_k);
            return this->Retrieve(search_request);
        }

        /**
         * Search with fine-grained search request
         * @param search_request
         * @return
         */
        [[nodiscard]] virtual AsyncIterator<Document> Retrieve(const SearchRequest& search_request) const = 0;

        [[nodiscard]] StepFunctionPtr AsContextRetrieverFunction(const RetrieverFunctionOptions& options = {}) const {
            return std::make_shared<LambdaStepFunction>([&, options](const JSONContextPtr &ctx) {
                    const auto question_string = ctx->RequirePrimitive<std::string>();
                    const auto doc_itr = Retrieve({.text = question_string, .top_k = options.top_k});
                    std::string context_string = DocumentUtils::CombineDocuments(doc_itr);
                    ctx->ProducePrimitive(context_string);
                    return ctx;
                }
            );
        }

    };

    using RetrieverPtr = std::shared_ptr<BaseRetriever>;

    class BaseStatefulRetriever : public BaseRetriever, public IStatefulRetriever {
    public:

        void Ingest(const AsyncIterator<Document> &input) override = 0;

        /**
         * Return the underlying DocStore for original documents
         * @return
         */
        virtual DocStorePtr GetDocStore() = 0;

    };

    using StatefulRetrieverPtr = std::shared_ptr<BaseStatefulRetriever>;

}


#endif //INSTINCT_BASERETRIEVER_HPP
