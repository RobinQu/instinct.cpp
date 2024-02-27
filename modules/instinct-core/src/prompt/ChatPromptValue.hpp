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

namespace INSTINCT_CORE_NS {
    class ChatPromptValue : public PromptValue {
        MessageVariants messages_;

    public:
        explicit ChatPromptValue(MessageVariants messages)
            : PromptValue("ChatPromptValue"), messages_(std::move(messages)) {
        }

        [[nodiscard]] std::string ToString() const override;

        [[nodiscard]] std::vector<MessageVariant> ToMessages() const override;
    };

    inline std::string ChatPromptValue::ToString() const {
        auto parts = messages_ | std::views::transform([](const MessageVariant& m) {
            return std::visit([](const auto& v) -> std::string { return v.ToString(); }, m);
        });
        return langchian::core::StringUtils::JoinWith(parts, "\n");
    }

    inline MessageVariants ChatPromptValue::ToMessages() const {
        return messages_;
    }
} // core
// langchain

#endif //CHATPROMPTVALUE_H
