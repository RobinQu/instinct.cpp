//
// Created by RobinQu on 2024/1/10.
//

#ifndef FUNCTIONMESSAGE_H
#define FUNCTIONMESSAGE_H

#include "BaseMessage.h"
#include <string>
#include "CoreGlobals.h"

namespace LC_CORE_NS {

class FunctionMessage: public BaseMessage  {

    std::string name;
public:
    FunctionMessage(std::string content,  std::string name)
        : BaseMessage(std::move(content), kFunctionMessageType),
          name(std::move(name)) {
    }

    std::string ToString() override;
};

} // core
// langchian

#endif //FUNCTIONMESSAGE_H
