//
// Created by RobinQu on 2024/1/12.
//

#ifndef LLMGENERATION_H
#define LLMGENERATION_H
#include <string>
#include "../types.h"


namespace langchain {

class LLMGeneration {
private:
public:
    const std::string text;
    const OptionDict generation_info;
    const std::string type;

    LLMGeneration(std::string text, OptionDict generation_info)
        : text(std::move(text)),
          generation_info(std::move(generation_info)),
          type("Generation") {
    }
};

using LLMGenerationPtr = std::shared_ptr<LLMGeneration>;

} // langchain

#endif //LLMGENERATION_H
