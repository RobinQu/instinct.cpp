//
// Created by RobinQu on 2024/1/10.
//

#ifndef FUNCTIONMESSAGE_H
#define FUNCTIONMESSAGE_H

#include "Message.hpp"
#include <string>
#include "CoreGlobals.hpp"

LC_CORE_NS {
    class FunctionMessage : public Message {
        std::string name;

    public:
        FunctionMessage(std::string content, std::string name)
            : Message(std::move(content), kFunctionMessageType),
              name(std::move(name)) {
        }

        std::string ToString() override;
    };

    inline std::string FunctionMessage::ToString() {
        return fmt::format("{role}: {content}", fmt::arg("role", "Function"), fmt::arg("content", GetContent()));
    }
} // core
// langchian

#endif //FUNCTIONMESSAGE_H
