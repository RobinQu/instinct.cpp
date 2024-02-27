//
// Created by RobinQu on 2024/2/14.
//

#ifndef HUMANMESSAGEPROMPTTEMPLATE_H
#define HUMANMESSAGEPROMPTTEMPLATE_H

#include "BaseStringMessagePromptTemplate.hpp"
#include "CoreGlobals.hpp"
#include "CoreTypes.hpp"
#include "PlainPromptTemplate.hpp"

namespace INSTINCT_CORE_NS {
    class HumanMessagePromptTemplate : public BaseStringMessagePromptTemplate {
    public:
        explicit HumanMessagePromptTemplate(PlainPromptTemplate prompt)
            : BaseStringMessagePromptTemplate(std::move(prompt)) {
        }

        MessageVariant Format(const OptionDict& variables) override;
    };

    inline MessageVariant HumanMessagePromptTemplate::Format(const OptionDict& variables) {
        return HumanMessage(prompt_.Format(variables));
    }
} // namespace INSTINCT_CORE_NS

#endif //HUMANMESSAGEPROMPTTEMPLATE_H
