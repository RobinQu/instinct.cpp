//
// Created by RobinQu on 2024/1/10.
//

#ifndef SYSTEMMESSAGE_H
#define SYSTEMMESSAGE_H

#include "BaseMessage.h"
#include <string>


namespace langchain {
namespace core {

class SystemMessage: public BaseMessage {
public:
    explicit SystemMessage(std::string content)
        : BaseMessage(std::move(content), kSystemMessageType) {
    }
};

} // core
} // langchain

#endif //SYSTEMMESSAGE_H
