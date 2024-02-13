//
// Created by RobinQu on 2024/2/13.
//

#ifndef AIMESSAGEPROMPTTEMPLATE_H
#define AIMESSAGEPROMPTTEMPLATE_H

#include "BaseMessagePromptTemplate.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {
    class AIMessagePromptTemplate: public BaseMessagePromptTemplate {
    public:
        explicit AIMessagePromptTemplate(const PromptTemplatePtr& prompt)
            : BaseMessagePromptTemplate(prompt) {
        }

        BaseMessagePtr Format(const OptionDict& variables) override;

        BaseMessagePtr Format() override;
    };
}

#endif //AIMESSAGEPROMPTTEMPLATE_H
