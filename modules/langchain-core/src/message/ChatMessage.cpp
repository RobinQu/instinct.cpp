//
// Created by RobinQu on 2024/1/10.
//

#include "ChatMessage.h"
#include <fmt/format.h>

namespace langchain::core {

    std::string ChatMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", role), fmt::arg("content", GetContent()));
    }
} // core
// langchain