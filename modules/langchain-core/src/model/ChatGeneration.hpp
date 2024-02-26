//
// Created by RobinQu on 2024/2/12.
//

#ifndef CHATGENERATION_H
#define CHATGENERATION_H


#include <string>

#include "Generation.hpp"
#include "message/ChatMessage.hpp"

LC_CORE_NS {

struct ChatGeneration: Generation {
    ChatMessage message;
    // ChatGeneration(std::string text, ChatMessage message, OptionDict generation_info)
    //     : Generation(std::move(text), std::move(generation_info), "ChatGeneration"),
    //       message(std::move(message)) {
    // }
};
using ChatGenerationPtr = std::shared_ptr<ChatGeneration>;

} // core
// langchain

#endif //CHATGENERATION_H
