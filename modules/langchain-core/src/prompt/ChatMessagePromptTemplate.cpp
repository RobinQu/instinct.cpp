//
// Created by RobinQu on 2024/2/13.
//

#include "ChatMessagePromptTemplate.h"
#include "CoreGlobals.h"
#include "message/ChatMessage.h"


namespace LC_CORE_NS {
    ChatMessagePromptTemplate::ChatMessagePromptTemplate(std::string role, const PromptTemplatePtr& prompt_template): BaseMessagePromptTemplate(prompt_template), role_(std::move(role)) {
    }

    BaseMessagePtr ChatMessagePromptTemplate::Format(const OptionDict& variables) {
        return std::make_shared<ChatMessage>(prompt_->Format(), role_);
    }

    BaseMessagePtr ChatMessagePromptTemplate::Format() {
        return Format({});
    }
}
