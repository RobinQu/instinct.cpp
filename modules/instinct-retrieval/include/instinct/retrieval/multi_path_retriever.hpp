//
// Created by RobinQu on 2024/5/26.
//

#ifndef MUTLI_PATH_RETRIEVER_HPP
#define MUTLI_PATH_RETRIEVER_HPP
#include <rpp/rpp.hpp>
#include <instinct/retrieval/base_retriever.hpp>
#include <instinct/ranker/base_ranking_model.hpp>


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     * This retriever will attempt to perform retrieving actions with multiple children retrievers, and re-rank recalled documents using a `RankingModel`, generating a collection of at most `top_k` documents.
     */
    class MultiPathRetriever final: public BaseRetriever {
        RankingModelPtr ranking_model_;
        std::vector<RetrieverPtr> retrievers_;
        ThreadPool& thread_pool_;

    public:
        MultiPathRetriever(
            RankingModelPtr ranking_model,
            std::vector<RetrieverPtr> retrievers,
            ThreadPool& thread_pool
        )
            : ranking_model_(std::move(ranking_model)),
              retrievers_(std::move(retrievers)),
              thread_pool_(thread_pool) {
            assert_gt(retrievers_.size(), 0, "should have at least one retriever");
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const SearchRequest &search_request) const override {
            using namespace std::chrono_literals;

            if(retrievers_.size() == 1) return retrievers_[0]->Retrieve(search_request);

            return rpp::source::create<Document>([&, search_request](const auto& observer) {
                auto multi_futures = thread_pool_.submit_sequence<size_t>(0, retrievers_.size(), [&](const size_t idx) {
                    return CollectVector(retrievers_[idx]->Retrieve(search_request));
                });
                if (!multi_futures.wait_for(60s)) {
                    throw InstinctException("Retrieving with multiple retrievers has been timeout");
                }

                std::vector<Document> docs;
                for (auto& f: multi_futures) {
                    const auto& batch =f.get();
                    docs.insert(docs.end(), batch.begin(), batch.end());
                }
                const auto dedupe_result = std::ranges::unique(docs, {}, &Document::id);
                docs.erase(dedupe_result.begin(), dedupe_result.end());
                LOG_DEBUG("unique_docs.size()={},top_k={}", docs.size(), search_request.top_k());
                if (docs.size()>search_request.top_k()) {
                    const auto idx_with_score = ranking_model_->RerankDocuments(docs, search_request.query(), search_request.top_k());
                    LOG_DEBUG("Re-ranked docs: {}", DocumentUtils::StringifyRerankResult(idx_with_score));
                    for(int i=0;i<search_request.top_k() && i<idx_with_score.size();++i) {
                        if (const auto&[idx, score] = idx_with_score[i]; idx < docs.size()) {
                            observer.on_next(docs.at(idx));
                        }
                    }
                } else {
                    for(const auto& doc: docs) {
                        observer.on_next(doc);
                    }
                }
                observer.on_completed();
            });
        }

    };


    template<typename ...R>
    static RetrieverPtr CreateMultiPathRetriever(RankingModelPtr ranking_model, R... r) {
        std::vector<RetrieverPtr> retrievers {r...};
        return std::make_shared<MultiPathRetriever>(ranking_model, retrievers, COMPUTE_WORKER_POOL);
    }
}



#endif //MUTLI_PATH_RETRIEVER_HPP
