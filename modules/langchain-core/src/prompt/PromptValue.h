//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPTVALUE_H
#define PROMPTVALUE_H
#include <memory>

#include "../message/BaseMessage.h"

namespace langchain {
namespace core {

class PromptValue {
    std::string type;
public:
    explicit PromptValue(std::string type)
        : type(std::move(type)) {
    }

    virtual ~PromptValue() = 0;
    virtual std::string ToString() = 0;
    virtual BaseMessagePtr ToMessage() = 0;
};
using PromptValuePtr = std::shared_ptr<PromptValue>;

} // core
} // langchain

#endif //PROMPTVALUE_H
