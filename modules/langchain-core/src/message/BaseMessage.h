//
// Created by RobinQu on 2024/1/10.
//

#ifndef BASEMESSAGE_H
#define BASEMESSAGE_H
#include <memory>
#include <string>
#include "MessageType.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

class BaseMessage {

    std::string content_;
    MessageType type_;

public:
    BaseMessage(std::string content, const MessageType type)
        : content_(std::move(content)),
          type_(type) {
    }

    virtual ~BaseMessage() = default;
    [[nodiscard]] std::string GetContent() const;
    [[nodiscard]] MessageType GetType() const;
    virtual std::string ToString() = 0;

};
using BaseMessagePtr = std::shared_ptr<BaseMessage>;

} // core
// langchain

#endif //BASEMESSAGE_H
