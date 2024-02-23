//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "Message.hpp"
#include <string>

LC_CORE_NS {
    class ChatMessage : public Message {
        std::string role_;

    public:

        ChatMessage(std::string content, std::string role)
            : Message(std::move(content), kChatMessageType),
              role_(std::move(role)) {
        }

        [[nodiscard]] std::string ToString() const override ;

        [[nodiscard]] std::string GetRole() const {
            return role_;
        }
    };

    inline std::string ChatMessage::ToString() const {
        return fmt::format("{role}: {content}", fmt::arg("role", role_), fmt::arg("content", GetContent()));
    }
} // core
// langchain

#endif //CHATMESSAGE_H
