//
// Created by RobinQu on 2024/2/13.
//

#ifndef BASESTRINGMESSAGEPROMPTTEMPLATE_H
#define BASESTRINGMESSAGEPROMPTTEMPLATE_H
#include "BasePromptTemplate.h"
#include "CoreGlobals.h"
#include "PromptTemplate.h"

namespace LC_CORE_NS {
    class BaseMessagePromptTemplate {
    protected:
        PromptTemplatePtr prompt_;

    public:
        explicit BaseMessagePromptTemplate(const PromptTemplatePtr& prompt);

        virtual ~BaseMessagePromptTemplate() = 0;

        virtual BaseMessagePtr Format(const OptionDict& variables) = 0;

        virtual BaseMessagePtr Format() = 0;
    };

    using BaseMessagePromptTemplatePtr = std::shared_ptr<BaseMessagePromptTemplate>;
} // LC_CORE_NS

#endif //BASESTRINGMESSAGEPROMPTTEMPLATE_H
