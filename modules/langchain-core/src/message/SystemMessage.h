//
// Created by RobinQu on 2024/1/10.
//

#ifndef SYSTEMMESSAGE_H
#define SYSTEMMESSAGE_H

#include "BaseMessage.h"
#include <string>

namespace LC_CORE_NS {

class SystemMessage: public BaseMessage {
public:
    explicit SystemMessage(std::string content)
        : BaseMessage(std::move(content), kSystemMessageType) {
    }

    std::string ToString() override;
};

} // core
// langchain

#endif //SYSTEMMESSAGE_H
