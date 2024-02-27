//
// Created by RobinQu on 2024/2/13.
//

#ifndef AIMESSAGEPROMPTTEMPLATE_H
#define AIMESSAGEPROMPTTEMPLATE_H

#include "BaseStringMessagePromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "message/AIMessage.hpp"

namespace INSTINCT_CORE_NS {
    class AIMessagePromptTemplate: public BaseStringMessagePromptTemplate {
    public:
        explicit AIMessagePromptTemplate(PlainPromptTemplate prompt)
            : BaseStringMessagePromptTemplate(std::move(prompt)) {
        }

        MessageVariant Format(const OptionDict& variables) override;
    };

    inline MessageVariant AIMessagePromptTemplate::Format(const OptionDict& variables) {
        return AIMessage(prompt_.Format(variables));
    }
}

#endif //AIMESSAGEPROMPTTEMPLATE_H
