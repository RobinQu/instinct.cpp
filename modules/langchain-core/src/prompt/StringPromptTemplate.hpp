//
// Created by RobinQu on 2024/1/9.
//

#ifndef STRINGPROMPTTEMPLATE_H
#define STRINGPROMPTTEMPLATE_H


#include "PromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "StringPromptValue.hpp"
#include "Forwards.hpp"

LC_CORE_NS {
    static const OptionDict EMPTY_TEMPLATE_VARIABLES = {};

    class StringPromptTemplate : public PromptTemplate {
    public:
        PromptValueVairant FormatPrompt(const OptionDict& variables) override;
    };

    inline PromptValueVairant StringPromptTemplate::FormatPrompt(const OptionDict& variables) {
        const auto string_value = Format(variables);
        return StringPromptValue{string_value};
    }
} // core
// langchain

#endif //STRINGPROMPTTEMPLATE_H
