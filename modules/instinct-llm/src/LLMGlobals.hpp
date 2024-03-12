//
// Created by RobinQu on 2024/2/13.
//

#ifndef MODELGLOBALS_H
#define MODELGLOBALS_H

#include "CoreGlobals.hpp"

#define INSTINCT_LLM_NS instinct::llm

namespace INSTINCT_LLM_NS {

    using TokenSize = unsigned long;
    using TokenId = unsigned long;

    static std::string DEFAULT_PROMPT_INPUT_KEY = "question";
    static std::string DEFAULT_ANSWER_OUTPUT_KEY = "answer";

}


#endif //MODELGLOBALS_H
