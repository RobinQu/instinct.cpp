//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "BaseMessage.h"
#include <string>


namespace langchain::core {

class ChatMessage: public BaseMessage {
    std::string role;
public:
    ChatMessage(std::string content,  std::string role)
        : BaseMessage(std::move(content), kChatMessageType),
          role(std::move(role)) {
    }
};

} // core
// langchain

#endif //CHATMESSAGE_H
