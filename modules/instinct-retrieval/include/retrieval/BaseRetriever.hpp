//
// Created by RobinQu on 2024/3/17.
//

#ifndef INSTINCT_BASERETRIEVER_HPP
#define INSTINCT_BASERETRIEVER_HPP

#include "retrieval/IRetriever.hpp"
#include "../../../instinct-core/include/tools/DocumentUtils.hpp"
#include "functional/StepFunctions.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    struct RetrieverFunctionOptions {
//        std::string text_query_variable_key = "query";
        int top_k = 10;
    };

    class BaseRetriever : public ITextRetriever {
    public:

        [[nodiscard]] AsyncIterator<Document> Retrieve(const TextQuery &query) const override = 0;

        [[nodiscard]] StepFunctionPtr AsContextRetrieverFunction(const RetrieverFunctionOptions& options = {}) const {
            return std::make_shared<LambdaStepFunction>([&](const JSONContextPtr &ctx) {
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
        [[nodiscard]] AsyncIterator<Document> Retrieve(const TextQuery &query) const override = 0;

        void Ingest(const AsyncIterator<Document> &input) override = 0;
    };

    using StatefulRetrieverPtr = std::shared_ptr<BaseStatefulRetriever>;

}


#endif //INSTINCT_BASERETRIEVER_HPP
