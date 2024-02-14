//
// Created by RobinQu on 2024/2/14.
//

#ifndef HUMANMESSAGEPROMPTTEMPLATE_H
#define HUMANMESSAGEPROMPTTEMPLATE_H

#include "BaseMessagePromptTemplate.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

class HumanMessagePromptTemplate: public BaseMessagePromptTemplate {
public:
    explicit HumanMessagePromptTemplate(const PromptTemplatePtr& prompt)
        : BaseMessagePromptTemplate(prompt) {
    }

    BaseMessagePtr Format(const OptionDict& variables) override;

    BaseMessagePtr Format() override;
};

} // LC_CORE_NS

#endif //HUMANMESSAGEPROMPTTEMPLATE_H
