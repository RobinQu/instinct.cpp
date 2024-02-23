//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPTVALUE_H
#define PROMPTVALUE_H
#include <memory>

#include "message/Message.hpp"
#include "prompt/Forwards.hpp"


LC_CORE_NS {

class PromptValue {
    std::string type;
public:

    explicit PromptValue(std::string type)
        : type(std::move(type)) {
    }

    virtual ~PromptValue() = default;
    virtual std::string ToString() = 0;
    virtual std::vector<MessageVariant> ToMessages() = 0;
};
using PromptValuePtr = std::shared_ptr<PromptValue>;

} // core

#endif //PROMPTVALUE_H
