//
// Created by RobinQu on 2024/2/14.
//

#include "SystemMessagePromptTemplate.h"

#include "message/SystemMessage.h"

namespace LC_CORE_NS {
    BaseMessagePtr SystemMessagePromptTemplate::Format(const OptionDict& variables) {
        return std::make_shared<SystemMessage>(prompt_->Format(variables));
    }

    BaseMessagePtr SystemMessagePromptTemplate::Format() {
        return Format({});
    }
} // LC_CORE_NS