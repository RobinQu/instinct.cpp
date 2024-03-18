//
// Created by RobinQu on 2024/3/11.
//

#ifndef CHAINCONTEXTAWARE_HPP
#define CHAINCONTEXTAWARE_HPP

#include "ContextMutator.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {


    class IChainContextAware {
    public:
        IChainContextAware()=default;
        virtual ~IChainContextAware()=default;
        IChainContextAware(const IChainContextAware&)=delete;
        IChainContextAware(IChainContextAware&&)=delete;

        virtual std::vector<std::string> GetInputKeys() = 0;
        virtual std::vector<std::string> GetOutputKeys() = 0;
        virtual void EnhanceContext(const ContextMutataorPtr& context_mutataor) = 0;

        void ValidateInput(const ContextPtr& input) {
            for (const auto& k: this->GetInputKeys()) {
                assert_true(input->values().contains(k), "context should contain key " + k);
            }
        }
    };

}


#endif //CHAINCONTEXTAWARE_HPP
