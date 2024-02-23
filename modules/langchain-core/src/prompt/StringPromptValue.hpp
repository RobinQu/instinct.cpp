//
// Created by RobinQu on 2024/1/10.
//

#ifndef STRINGPROMPTVALUE_H
#define STRINGPROMPTVALUE_H

#include <string>
#include "PromptValue.hpp"
#include "CoreGlobals.hpp"
#include "Forwards.hpp"
#include "message/HumanMessage.hpp"

LC_CORE_NS {

class StringPromptValue: public PromptValue {
    std::string text;
public:
    explicit StringPromptValue(std::string text)
        : PromptValue("StringPromptValue"), text(std::move(text)) {
    }

    std::string ToString() override;

    std::vector<MessageVariant> ToMessages() override;
};

    inline std::string StringPromptValue::ToString() {
        return text;
    }

    inline std::vector<MessageVariant> StringPromptValue::ToMessages() {
        return {HumanMessage(text)};
    }


} // core
// langchain

#endif //STRINGPROMPTVALUE_H
