//
// Created by RobinQu on 2024/1/10.
//

#ifndef HUMANMESSAGE_H
#define HUMANMESSAGE_H

#include "Message.hpp"
#include <string>
#include "CoreGlobals.hpp"


LC_CORE_NS {
    class HumanMessage : public Message {
        bool exmaple;

    public:
        explicit HumanMessage(std::string content, const bool exmaple = false)
            : Message(std::move(content), kHumanMessageType),
              exmaple(exmaple) {
        }

        [[nodiscard]] std::string ToString() const override;
    };

    inline std::string HumanMessage::ToString() const {
        return fmt::format("{role}: {content}", fmt::arg("role", "Human"), fmt::arg("content", GetContent()));
    }
} // core
// langchain

#endif //HUMANMESSAGE_H
