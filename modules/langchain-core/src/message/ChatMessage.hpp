//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "Message.hpp"
#include <string>

LC_CORE_NS {
    class ChatMessage : public Message {
        std::string role;

    public:
        ChatMessage(std::string content, std::string role)
            : Message(std::move(content), kChatMessageType),
              role(std::move(role)) {
        }

        std::string ToString() override;
    };

    inline std::string ChatMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", role), fmt::arg("content", GetContent()));
    }
} // core
// langchain

#endif //CHATMESSAGE_H
