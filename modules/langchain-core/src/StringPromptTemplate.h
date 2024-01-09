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
    PromptValue* FormatPrompt() override;
};

} // core
} // langchain

#endif //STRINGPROMPTTEMPLATE_H
