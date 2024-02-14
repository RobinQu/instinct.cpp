//
// Created by RobinQu on 2024/1/10.
//

#include "FunctionMessage.h"


namespace langchain::core {
    std::string FunctionMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "Function"), fmt::arg("content", GetContent()));
    }
} // core
// langchian