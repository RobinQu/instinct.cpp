//
// Created by RobinQu on 2024/5/26.
//

#ifndef IRANKINGMODEL_HPP
#define IRANKINGMODEL_HPP

#include <instinct/llm_global.hpp>

namespace INSTINCT_LLM_NS {
    class IRankingModel {
    public:
        IRankingModel()=default;
        virtual ~IRankingModel()=default;
        IRankingModel(const IRankingModel&)=delete;
        IRankingModel(IRankingModel&&)=delete;

        virtual float GetRankingScore(const std::string& query, const std::string& doc) = 0;
    };

}


#endif //IRANKINGMODEL_HPP
