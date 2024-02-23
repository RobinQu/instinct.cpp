//
// Created by RobinQu on 2024/1/10.
//

#ifndef SYSTEMMESSAGE_H
#define SYSTEMMESSAGE_H

#include "Message.hpp"
#include <string>

LC_CORE_NS {
    class SystemMessage : public Message {
    public:
        explicit SystemMessage(std::string content)
            : Message(std::move(content), kSystemMessageType) {
        }

        std::string ToString() override;
    };

    inline std::string SystemMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "System"), fmt::arg("content", GetContent()));
    }
} // core
// langchain

#endif //SYSTEMMESSAGE_H
