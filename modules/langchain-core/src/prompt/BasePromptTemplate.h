//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <map>
#include <string>
#include <filesystem>

#include "../document/Document.h"
#include "PromptValue.h"

namespace langchain {
namespace core {


class BasePromptTemplate {
    // std::map<std::string,std::any> ;
public:
    virtual  ~BasePromptTemplate();
    virtual PromptValuePtr FormatPrompt() = 0;
    virtual std::string FormatDocument(DocumentPtr document) = 0;
    virtual void Save(std::filesystem::path filepath) = 0;

};

} // model
} // langchain

#endif //PROMPT_H
