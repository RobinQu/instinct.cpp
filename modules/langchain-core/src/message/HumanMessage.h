//
// Created by RobinQu on 2024/1/10.
//

#ifndef HUMANMESSAGE_H
#define HUMANMESSAGE_H

#include "BaseMessage.h"
#include <string>


namespace langchain {
namespace core {

class HumanMessage: public BaseMessage {
    bool exmaple = false;
public:
    HumanMessage(std::string content,  const bool exmaple)
        : BaseMessage(std::move(content), kHumanMessageType),
          exmaple(exmaple) {
    }
};

} // core
} // langchain

#endif //HUMANMESSAGE_H
