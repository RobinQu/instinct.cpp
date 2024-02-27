//
// Created by RobinQu on 2024/2/12.
//

#ifndef CHATRESULT_H
#define CHATRESULT_H

#include "ChatGeneration.hpp"
#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {

struct ChatResult {
    std::vector<ChatGeneration> generations;
    OptionDict llm_output;
};
using ChatResultPtr = std::shared_ptr<ChatResult>;

} // core
// langchain

#endif //CHATRESULT_H
