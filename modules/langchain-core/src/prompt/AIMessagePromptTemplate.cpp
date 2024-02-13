//
// Created by RobinQu on 2024/2/13.
//

#include "AIMessagePromptTemplate.h"

#include "message/AIMessage.h"


namespace LC_CORE_NS {
    BaseMessagePtr AIMessagePromptTemplate::Format(const OptionDict& variables) {
        return std::make_shared<AIMessage>(prompt_->Format(variables));
    }

    BaseMessagePtr AIMessagePromptTemplate::Format() {
        return Format({});
    }
}
