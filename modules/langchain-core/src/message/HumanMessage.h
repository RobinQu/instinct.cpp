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
    bool exmaple;
public:
    explicit HumanMessage(std::string content,  const bool exmaple = false)
        : BaseMessage(std::move(content), kHumanMessageType),
          exmaple(exmaple) {
    }

    std::string GetContent() override;

    std::string GetType() override;

    std::string ToString() override;
};

} // core
} // langchain

#endif //HUMANMESSAGE_H
