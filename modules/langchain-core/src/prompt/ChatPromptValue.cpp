//
// Created by RobinQu on 2024/1/10.
//

#include "ChatPromptValue.h"
#include <ranges>

#include "tools/StringUtils.h"

namespace LC_CORE_NS {
    std::string ChatPromptValue::ToString() {
        auto parts = messages_ | std::views::transform([](const BaseMessagePtr& m) {return m->ToString();});
        return langchian::core::StringUtils::Join({parts.begin(), parts.end()});
    }

    std::vector<BaseMessagePtr> ChatPromptValue::ToMessages() {
        return messages_;
    }
} // core
// langchain