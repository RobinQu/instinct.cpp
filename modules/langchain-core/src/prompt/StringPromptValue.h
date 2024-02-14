//
// Created by RobinQu on 2024/1/10.
//

#ifndef STRINGPROMPTVALUE_H
#define STRINGPROMPTVALUE_H

#include <string>
#include "PromptValue.h"
#include "CoreGlobals.h"


namespace LC_CORE_NS {

class StringPromptValue: public PromptValue {
    std::string text;
public:
    explicit StringPromptValue(std::string text)
        : PromptValue("StringPromptValue"), text(std::move(text)) {
    }

    std::string ToString() override;

    std::vector<BaseMessagePtr> ToMessages() override;
};


} // core
// langchain

#endif //STRINGPROMPTVALUE_H
