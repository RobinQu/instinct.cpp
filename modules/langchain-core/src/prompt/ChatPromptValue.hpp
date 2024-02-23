//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTVALUE_H
#define CHATPROMPTVALUE_H

#include <string>
#include "PromptValue.hpp"
#include <vector>
#include "CoreGlobals.hpp"
#include "tools/StringUtils.hpp"
#include "Forwards.hpp"

LC_CORE_NS {
    class ChatPromptValue : public PromptValue {
        MessageVariants messages_;

    public:
        explicit ChatPromptValue(MessageVariants messages)
            : PromptValue("ChatPromptValue"), messages_(std::move(messages)) {
        }

        std::string ToString() override;

        std::vector<MessageVariant> ToMessages() override;
    };

    inline std::string ChatPromptValue::ToString() {
        auto parts = messages_ | std::views::transform([](const MessageVariant& m) {
            return std::visit([](auto&& v) { v.ToString(); }, m);
        });
        return langchian::core::StringUtils::JoinWith({parts.begin(), parts.end()}, "\n");
    }

    inline MessageVariants ChatPromptValue::ToMessages() {
        return messages_;
    }
} // core
// langchain

#endif //CHATPROMPTVALUE_H
