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

        // virtual void SaveMemory(const PromptValue& prompt_value, const Generation& generation) = 0;


        virtual void SaveMemory(const ContextPtr& context) = 0;


        // virtual void SaveMemory(const std::string& prompt, const std::string& answer);

        // virtual void SaveMemory(const MessageList& messages, const Message& answer);

        virtual void LoadMemories(const ContextMutataorPtr& bulder) = 0;


    };
}

#endif //CHATMEMORY_HPP
