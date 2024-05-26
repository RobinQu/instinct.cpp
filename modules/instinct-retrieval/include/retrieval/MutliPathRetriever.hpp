//
// Created by RobinQu on 2024/5/26.
//

#ifndef MUTLI_PATH_RETRIEVER_HPP
#define MUTLI_PATH_RETRIEVER_HPP
#include <rpp/rpp.hpp>


#include "BaseRetriever.hpp"
#include "ranker/BaseRankingModel.hpp"


namespace INSTINCT_RETRIEVAL_NS {
    using namespace INSTINCT_LLM_NS;

    /**
     * This retriever will attempt to perform retrieving actions with multiple children retrievers, and re-rank recalled documents using a `RankingModel`, generating a collection of at most `top_k` documents.
     */
    class MultiPathRetriever final: public BaseRetriever {
        RankingModelPtr ranking_model_;
        std::vector<RetrieverPtr> retrievers_;
        ThreadPoolPtr thread_pool_;

    public:
        MultiPathRetriever(
            RankingModelPtr ranking_model,
            std::vector<RetrieverPtr> retrievers,
            ThreadPoolPtr thread_pool
        )
            : ranking_model_(std::move(ranking_model)),
              retrievers_(std::move(retrievers)),
              thread_pool_(std::move(thread_pool)){
            assert_gt(retrievers_.size(), 0, "should have at least one retriever");
        }

        [[nodiscard]] AsyncIterator<Document> Retrieve(const TextQuery &query) const override {
            using namespace std::chrono_literals;

            if(retrievers_.size() == 1) return retrievers_[0]->Retrieve(query);

            auto multi_futures = thread_pool_->submit_sequence<size_t>(0, retrievers_.size(), [&](const size_t idx) {
                return CollectVector(retrievers_[idx]->Retrieve(query));
            });
            if (!multi_futures.wait_for(60s)) {
                throw InstinctException("Retrieving with multiple retrievers has been timeout");
            }

            std::vector<Document> docs;
            for (auto& f: multi_futures) {
                const auto& batch =f.get();
                docs.insert(docs.end(), batch.begin(), batch.end());
            }

            auto multi_futures_2 = thread_pool_->submit_sequence<size_t>(0, docs.size(), [&](const size_t idx) {
                return ranking_model_->GetRankingScore(query.text, docs[idx].text());
            });
            std::vector<std::pair<std::string, float>> doc_id_with_score;
            doc_id_with_score.reserve(docs.size());
            for(int i=0; auto& f: multi_futures_2) {
                doc_id_with_score.emplace_back(docs[i++].id(), f.get());
            }
            std::sort(doc_id_with_score.begin(), doc_id_with_score.end(), [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

            std::map<std::string, size_t> doc_id_idx;
            for(int i=0;i<docs.size();++i) {
                doc_id_idx[docs[i].id()] = i;
            }

            return rpp::source::create<Document>([&](const auto& observer) {
                for(int i=0;i<query.top_k && i<doc_id_with_score.size();++i) {
                    const auto&[doc_id, score] = doc_id_with_score[i];
                    observer.on_next(docs.at(doc_id_idx.at(doc_id)));
                }
            });
        }
    };
}



#endif //MUTLI_PATH_RETRIEVER_HPP
