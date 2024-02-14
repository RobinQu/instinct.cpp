//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <map>
#include <string>
#include <filesystem>
#include <fmt/format.h>

#include "CoreTypes.h"
#include "../document/Document.h"
#include "PromptValue.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {


class BasePromptTemplate {
    // std::map<std::string,std::any> ;
public:
    virtual ~BasePromptTemplate() = default;

    virtual PromptValuePtr FormatPrompt() = 0;
    virtual PromptValuePtr FormatPrompt(const OptionDict& variables) = 0;
    virtual std::string Format() = 0;
    virtual std::string Format(const OptionDict& variables) = 0;
    // virtual std::string FormatDocument(DocumentPtr document) = 0;
    // virtual void Save(std::filesystem::path filepath) = 0;

};

} // model
// langchain

#endif //PROMPT_H
