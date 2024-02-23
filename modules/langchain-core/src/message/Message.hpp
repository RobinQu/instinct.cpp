//
// Created by RobinQu on 2024/1/10.
//

#ifndef BASEMESSAGE_H
#define BASEMESSAGE_H
#include <memory>
#include <string>
#include "CoreGlobals.hpp"

LC_CORE_NS {
    enum MessageType {
        kUnspecified = 0,
        kChatMessageType,
        kSystemMessageType,
        kHumanMessageType,
        kFunctionMessageType,
        kAIMessageType,
        kToolMessageType
    };


    class Message {
        std::string content_;
        MessageType type_;

    public:
        Message(std::string content, const MessageType type)
            : content_(std::move(content)),
              type_(type) {
        }

        virtual ~Message() = default;

        [[nodiscard]] std::string GetContent() const;

        [[nodiscard]] MessageType GetType() const;

        virtual std::string ToString() = 0;
    };

    using BaseMessagePtr = std::shared_ptr<Message>;

    inline std::string Message::GetContent() const {
        return content_;
    }

    inline MessageType Message::GetType() const {
        return type_;
    }


} // core
// langchain

#endif //BASEMESSAGE_H
