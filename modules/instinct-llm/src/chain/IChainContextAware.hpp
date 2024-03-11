//
// Created by RobinQu on 2024/3/11.
//

#ifndef CHAINCONTEXTAWARE_HPP
#define CHAINCONTEXTAWARE_HPP
#include "ChainContextBuilder.hpp"


namespace INSTINCT_LLM_NS {

    class IChainContextAware {
    public:
        IChainContextAware()=default;
        virtual ~IChainContextAware()=default;
        IChainContextAware(const IChainContextAware&)=delete;
        IChainContextAware(IChainContextAware&&)=delete;

        virtual std::vector<std::string> GetInputKeys() = 0;
        virtual std::vector<std::string> GetOutputKeys() = 0;
        virtual void EnhanceContext(const ChainContextBuilderPtr& builder) = 0;
    };

}


#endif //CHAINCONTEXTAWARE_HPP
