//
// Created by RobinQu on 2024/1/12.
//

#ifndef LLMRESULT_H
#define LLMRESULT_H
#include <vector>

#include "Generation.h"


namespace langchain::core {

class LLMResult {
public:
    std::vector<std::vector<Generation>> generations;
    OptionDict llm_output;
    std::vector<Generation> flatten();
};

using LLMResultPtr = std::shared_ptr<LLMResult>;

} // core
// langchain

#endif //LLMRESULT_H
