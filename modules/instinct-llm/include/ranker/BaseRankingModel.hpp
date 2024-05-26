//
// Created by RobinQu on 2024/5/26.
//

#ifndef BASE_RANKER_HPP
#define BASE_RANKER_HPP

#include "LLMGlobals.hpp"
#include "functional/BaseRunnable.hpp"
#include "model/IRankingModel.hpp"

namespace INSTINCT_LLM_NS {

    struct QAPair {
        std::string query;
        std::string doc;
    };

    class BaseRankingModel:
        public virtual IRankingModel,
        public BaseRunnable<QAPair, float>,
        public std::enable_shared_from_this<BaseRankingModel> {
    public:
    float Invoke(const QAPair &input) override {
        return this->GetRankingScore(input.query, input.doc);
    }

    using RankingModel = std::shared_ptr<BaseRankingModel>;
};
}


#endif //BASE_RANKER_HPP
