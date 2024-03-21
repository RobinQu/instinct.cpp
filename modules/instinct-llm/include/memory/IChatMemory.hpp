//
// Created by RobinQu on 2024/3/9.
//

#ifndef CHATMEMORY_HPP
#define CHATMEMORY_HPP


#include "LLMGlobals.hpp"
#include <llm.pb.h>

#include "chain/IChainContextAware.hpp"

namespace INSTINCT_LLM_NS {

    class IChatMemory {
    public:
        IChatMemory()=default;
        virtual ~IChatMemory() = default;
        IChatMemory(const IChatMemory&)=delete;
        IChatMemory(IChatMemory&&)=delete;

        virtual void SaveMemory(const JSONContextPtr& context) = 0;
        virtual void LoadMemories(const JSONContextPtr& context) = 0;
    };
}

#endif //CHATMEMORY_HPP
