//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H

#include "BasePromptTemplate.h"
#include <string>
#include <vector>

namespace langchain {
namespace core {

class ChatPromptTemplate: BasePromptTemplate {

    std::vector<std::string> input_variables;
    std::vector<std::string> messages;
    bool validate_template = false;





};

} // core
} // langchain

#endif //CHATPROMPTTEMPLATE_H
