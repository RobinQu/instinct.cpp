//
// Created by RobinQu on 2024/1/10.
//

#ifndef FUNCTIONMESSAGE_H
#define FUNCTIONMESSAGE_H

#include "BaseMessage.h"
#include <string>

namespace langchain{
namespace core {

class FunctionMessage: BaseMessage  {

    std::string name;
public:
    FunctionMessage(std::string content,  std::string name)
        : BaseMessage(std::move(content), "function"),
          name(std::move(name)) {
    }
};

} // core
} // langchian

#endif //FUNCTIONMESSAGE_H
