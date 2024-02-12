//
// Created by RobinQu on 2024/1/15.
//

#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <string>
#include <boost/algorithm/string.hpp>

#include "message/BaseMessage.h"
#include "message/ChatMessage.h"
#include <boost/beast/core.hpp>
#include "CoreGlobals.h"


namespace LC_CORE_NS {
    static std::string GetBufferString(
        const std::vector<BaseMessagePtr>& messages,
        const std::string& human_prefix = "Human",
        const std::string& ai_preifx = "AI"
        ) {
        std::vector<std::string> string_messages;
        for(const auto& msg: messages) {
            string_messages.push_back(msg->ToString());
        }
        return boost::algorithm::join(string_messages, "\n");
    }
}

#endif //FUNCTIONS_H
