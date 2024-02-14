//
// Created by RobinQu on 2024/1/10.
//

#include "HumanMessage.h"


namespace LC_CORE_NS {

    std::string HumanMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "Human"), fmt::arg("content", GetContent()));
    }
} // core
// langchain