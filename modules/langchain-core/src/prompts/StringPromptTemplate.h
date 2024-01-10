//
// Created by RobinQu on 2024/1/9.
//

#ifndef STRINGPROMPTTEMPLATE_H
#define STRINGPROMPTTEMPLATE_H

#include "BasePromptTemplate.h"

namespace langchain {
namespace core {

class StringPromptTemplate: BasePromptTemplate {

public:
    PromptValuePtr FormatPrompt() override;

    std::string FormatDocument(DocumentPtr document) override;

    void Save(std::filesystem::path filepath) override;
};

} // core
} // langchain

#endif //STRINGPROMPTTEMPLATE_H
