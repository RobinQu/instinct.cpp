//
// Created by RobinQu on 2024/2/13.
//

#ifndef CHATMESSAGEPROMPTTEMPLATE_H
#define CHATMESSAGEPROMPTTEMPLATE_H
#include "BaseStringMessagePromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "message/ChatMessage.hpp"


LC_CORE_NS {
    class ChatMessagePromptTemplate : public BaseStringMessagePromptTemplate {
        std::string role_;

    public:
        ChatMessagePromptTemplate() = delete;

        ChatMessagePromptTemplate(std::string role,
                                  const PlainPromptTemplate& prompt_template): BaseStringMessagePromptTemplate(
            prompt_template), role_(std::move(role)) {
        }

        MessageVariant Format(const OptionDict& variables) override;
    };

    inline MessageVariant ChatMessagePromptTemplate::Format(const OptionDict& variables) {
        return ChatMessage(prompt_.Format(variables), role_);
    }
}


#endif //CHATMESSAGEPROMPTTEMPLATE_H
