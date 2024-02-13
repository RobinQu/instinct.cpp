//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTVALUE_H
#define CHATPROMPTVALUE_H

#include <string>
#include "PromptValue.h"
#include <vector>
#include "CoreGlobals.h"


namespace LC_CORE_NS {
    class ChatPromptValue : public PromptValue {
        std::vector<BaseMessagePtr> messages_;

    public:
        explicit ChatPromptValue(std::vector<BaseMessagePtr> messages)
            : PromptValue("ChatPromptValue"), messages_(std::move(messages)) {
        }

        std::string ToString() override;

        std::vector<BaseMessagePtr> ToMessages() override;
    };
} // core
// langchain

#endif //CHATPROMPTVALUE_H
