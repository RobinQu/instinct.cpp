//
// Created by RobinQu on 2024/2/14.
//

#include "HumanMessagePromptTemplate.h"

#include "message/HumanMessage.h"

namespace LC_CORE_NS {
    BaseMessagePtr HumanMessagePromptTemplate::Format(const OptionDict& variables) {
        return std::make_shared<HumanMessage>(prompt_->Format((variables)));
    }

    BaseMessagePtr HumanMessagePromptTemplate::Format() {
        return Format({});
    }
} // LC_CORE_NS