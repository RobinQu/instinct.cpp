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
#include "message/AIMessage.hpp"
#include "message/FunctionMessage.hpp"
#include "message/SystemMessage.hpp"
#include "message/ChatMessage.hpp"

namespace INSTINCT_CORE_NS {

class StringPromptValue: public PromptValue {
    std::string text;
public:
    explicit StringPromptValue(std::string text)
        : PromptValue("StringPromptValue"), text(std::move(text)) {
    }

    [[nodiscard]] std::string ToString() const override;

    [[nodiscard]] std::vector<MessageVariant> ToMessages() const override;
};

    inline std::string StringPromptValue::ToString() const {
        return text;
    }

    inline std::vector<MessageVariant> StringPromptValue::ToMessages() const {
        return {HumanMessage(text)};
    }


} // core
// langchain

#endif //STRINGPROMPTVALUE_H
