//
// Created by RobinQu on 2024/1/10.
//

#include "BaseMessage.h"


namespace LC_CORE_NS {
    std::string BaseMessage::GetContent() const {
        return content_;
    }

    MessageType BaseMessage::GetType() const {
        return type_;
    }
} // core
// langchain