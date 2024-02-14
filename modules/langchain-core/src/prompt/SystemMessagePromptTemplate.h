//
// Created by RobinQu on 2024/2/14.
//

#ifndef SYSTEMMESSAGEPROMPTTEMPLATE_H
#define SYSTEMMESSAGEPROMPTTEMPLATE_H

#include "BaseMessagePromptTemplate.h"
#include "CoreGlobals.h"

namespace LC_CORE_NS {

class SystemMessagePromptTemplate: public BaseMessagePromptTemplate{
public:
    explicit SystemMessagePromptTemplate(const PromptTemplatePtr& prompt)
        : BaseMessagePromptTemplate(prompt) {

    }

    BaseMessagePtr Format(const OptionDict& variables) override;

    BaseMessagePtr Format() override;
};

} // LC_CORE_NS

#endif //SYSTEMMESSAGEPROMPTTEMPLATE_H
