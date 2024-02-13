//
// Created by RobinQu on 2024/1/9.
//

#ifndef STRINGPROMPTTEMPLATE_H
#define STRINGPROMPTTEMPLATE_H



#include "BasePromptTemplate.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

// const auto a = fmt::arg("a", "b");

static const OptionDict EMPTY_TEMPLATE_VARIABLES = {};

class StringPromptTemplate: public BasePromptTemplate {

public:


    PromptValuePtr FormatPrompt(const OptionDict& variables) override;

    PromptValuePtr FormatPrompt() override;

    // std::string Format() override;
    //
    // std::string Format(const OptionDict& variables) override;

    // std::string FormatDocument(DocumentPtr document) override;
    //
    // void Save(std::filesystem::path filepath) override;
};

} // core
// langchain

#endif //STRINGPROMPTTEMPLATE_H
