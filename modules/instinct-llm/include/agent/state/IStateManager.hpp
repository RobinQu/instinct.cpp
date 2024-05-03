//
// Created by RobinQu on 2024/5/3.
//

#ifndef ISTATEMANAGER_HPP
#define ISTATEMANAGER_HPP

#include "LLMGlobals.hpp"

namespace INSTINCT_LLM_NS {
    class IStateManager {
    public:
        virtual ~IStateManager()=default;
        virtual std::future<AgentState> Load(const std::string& id) = 0;
        virtual std::future<void> Save(const AgentState& state) = 0;
    };

    using StateManagerPtr = std::shared_ptr<IStateManager>;
}


#endif //ISTATEMANAGER_HPP
