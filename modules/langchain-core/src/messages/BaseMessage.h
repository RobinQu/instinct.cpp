//
// Created by RobinQu on 2024/1/10.
//

#ifndef BASEMESSAGE_H
#define BASEMESSAGE_H
#include <memory>
#include <string>

namespace langchain {
namespace core {

class BaseMessage {
    std::string content;
    std::string type;

public:

    BaseMessage(std::string content, std::string type)
        : content(std::move(content)),
          type(std::move(type)) {
    }

    virtual ~BaseMessage() = 0;
    virtual std::string GetContent() = 0;
    virtual std::string GetType() = 0;

};
using BaseMessagePtr = std::shared_ptr<BaseMessage>;

} // core
} // langchain

#endif //BASEMESSAGE_H
