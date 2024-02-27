//
// Created by RobinQu on 2024/2/14.
//

#ifndef SYSTEMMESSAGEPROMPTTEMPLATE_H
#define SYSTEMMESSAGEPROMPTTEMPLATE_H

#include "BaseStringMessagePromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "message/SystemMessage.hpp"

namespace INSTINCT_CORE_NS {
    class SystemMessagePromptTemplate : public BaseStringMessagePromptTemplate {
    public:
        explicit SystemMessagePromptTemplate(PlainPromptTemplate prompt)
            : BaseStringMessagePromptTemplate(std::move(prompt)) {
        }

        MessageVariant Format(const OptionDict& variables) override;
    };


    inline MessageVariant SystemMessagePromptTemplate::Format(const OptionDict& variables) {
        const auto text = prompt_.Format(variables);
        return SystemMessage(text);
    }
} // namespace INSTINCT_CORE_NS

#endif //SYSTEMMESSAGEPROMPTTEMPLATE_H
