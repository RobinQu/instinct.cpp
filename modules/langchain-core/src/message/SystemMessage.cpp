//
// Created by RobinQu on 2024/1/10.
//

#include "SystemMessage.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

    std::string SystemMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "System"), fmt::arg("content", GetContent()));
    }
} // core
// langchain