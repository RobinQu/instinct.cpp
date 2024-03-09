//
// Created by RobinQu on 2024/3/9.
//

#ifndef CHATMEMORY_HPP
#define CHATMEMORY_HPP


#include "LLMGlobals.hpp"
#include <llm.pb.h>

namespace INSTINCT_LLM_NS {
    class IChatMemory {
    public:
        IChatMemory()=default;
        virtual ~IChatMemory() = default;
        IChatMemory(const IChatMemory&)=delete;
        IChatMemory(IChatMemory&&)=delete;

        virtual std::vector<std::string> GetMemoryKeys() = 0;
        virtual LLMChainContext GetMemroyVariables() = 0;
    };

    using IChatMemoryPtr = std::shared_ptr<IChatMemory>;
}

#endif //CHATMEMORY_HPP
