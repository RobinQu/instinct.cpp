//
// Created by RobinQu on 2024/2/12.
//

#ifndef CHATGENERATION_H
#define CHATGENERATION_H
#include <memory>
#include <string>

#include "Generation.h"
#include "message/ChatMessage.h"

namespace langchain::core {

class ChatGeneration: public Generation {
public:
    const ChatMessage message;

    ChatGeneration(std::string text, ChatMessage message, OptionDict generation_info)
        : Generation(std::move(text), std::move(generation_info), "ChatGeneration"),
          message(std::move(message)) {
    }
};
using ChatGenerationPtr = std::shared_ptr<ChatGeneration>;

} // core
// langchain

#endif //CHATGENERATION_H
