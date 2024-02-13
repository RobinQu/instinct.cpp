//
// Created by RobinQu on 2024/2/13.
//

#ifndef CHATMESSAGEPROMPTTEMPLATE_H
#define CHATMESSAGEPROMPTTEMPLATE_H
#include "CoreGlobals.h"
#include "BaseMessagePromptTemplate.h"


namespace LC_CORE_NS {
    class ChatMessagePromptTemplate: public BaseMessagePromptTemplate{
        std::string role_;
    public:
        ChatMessagePromptTemplate() = delete;
        ChatMessagePromptTemplate(std::string role, const PromptTemplatePtr& prompt_template);

        BaseMessagePtr Format(const OptionDict& variables) override;

        BaseMessagePtr Format() override;
    };

}




#endif //CHATMESSAGEPROMPTTEMPLATE_H
