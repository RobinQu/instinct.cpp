//
// Created by RobinQu on 2024/1/10.
//

#include "StringPromptValue.h"

#include "message/HumanMessage.h"


namespace LC_CORE_NS {
    std::string StringPromptValue::ToString() {
        return text;
    }

    std::vector<BaseMessagePtr> StringPromptValue::ToMessages() {
        return {std::make_shared<HumanMessage>(text)};
    }
} // core
// langchain