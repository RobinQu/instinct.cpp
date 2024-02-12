//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTVALUE_H
#define CHATPROMPTVALUE_H

#include <string>
#include "PromptValue.h"
#include <vector>

namespace langchain {
namespace core {

class ChatPromptValue: PromptValue {
    std::vector<std::string> messages;
public:
    explicit ChatPromptValue(std::vector<std::string> messages)
        : PromptValue("ChatPromptValue"), messages(std::move(messages)) {
    }

    std::string ToString() override;

    std::vector<BaseMessagePtr> ToMessages() override;
};

} // core
} // langchain

#endif //CHATPROMPTVALUE_H
