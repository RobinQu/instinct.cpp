//
// Created by RobinQu on 2024/1/15.
//

#include "AIMessage.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

    std::string AIMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "Asistant"), fmt::arg("content", GetContent()));
    }
} // core
// langchain