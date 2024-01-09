//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <map>
#include <string>
#include "PromptValue.h"

namespace langchain {
namespace core {


class BasePromptTemplate {
    // std::map<std::string,std::any> ;
public:
    virtual  ~BasePromptTemplate();
    virtual PromptValue* FormatPrompt() = 0;
};

} // model
} // langchain

#endif //PROMPT_H
