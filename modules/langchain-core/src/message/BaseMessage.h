//
// Created by RobinQu on 2024/1/10.
//

#ifndef BASEMESSAGE_H
#define BASEMESSAGE_H
#include <memory>
#include <string>
#include "MessageType.h"


namespace langchain::core {

class BaseMessage {
    std::string content;
    MessageType type;

public:
    BaseMessage(std::string content, const MessageType type)
        : content(std::move(content)),
          type(type) {
    }

    virtual ~BaseMessage() = default;
    virtual std::string GetContent() = 0;
    virtual std::string GetType() = 0;
    virtual std::string ToString() = 0;

};
using BaseMessagePtr = std::shared_ptr<BaseMessage>;

} // core
// langchain

#endif //BASEMESSAGE_H
