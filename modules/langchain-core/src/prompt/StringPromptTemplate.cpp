//
// Created by RobinQu on 2024/1/9.
//

#include "StringPromptTemplate.h"

#include "CoreGlobals.h"


#include "StringPromptValue.h"

namespace LC_CORE_NS {
    // StringPromptTemplate::StringPromptTemplate();

    PromptValuePtr StringPromptTemplate::FormatPrompt(const OptionDict& variables) {
        auto string_value = Format(variables);
        return std::make_shared<StringPromptValue>(string_value);
    }

    PromptValuePtr StringPromptTemplate::FormatPrompt() {
        return FormatPrompt({});
    }


}
