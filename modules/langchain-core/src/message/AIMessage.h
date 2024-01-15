//
// Created by RobinQu on 2024/1/15.
//

#ifndef AIMESSAGE_H
#define AIMESSAGE_H
#include "BaseMessage.h"

namespace langchain {
namespace core {

class AIMessage : public BaseMessage {

public:
    bool example;
    explicit AIMessage(std::string content, const bool example = false)
        : BaseMessage(std::move(content), kAIMessageType),
          example(example) {
    }
};

} // core
} // langchain

#endif //AIMESSAGE_H
