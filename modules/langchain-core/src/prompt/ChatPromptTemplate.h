//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H

#include "BasePromptTemplate.h"
#include <string>
#include <vector>

#include "BaseMessagePromptTemplate.h"


namespace LC_CORE_NS {
    class ChatPromptTemplate;
    using ChatPromptTemplatePtr = std::shared_ptr<ChatPromptTemplate>;


    class ChatPromptTemplate : public BasePromptTemplate {
        std::vector<BaseMessagePromptTemplatePtr> messages_;

    public:
        template<typename... Arg>
        static ChatPromptTemplatePtr FromMessages(const Arg&... args);

        explicit ChatPromptTemplate(std::vector<BaseMessagePromptTemplatePtr> messages);

        std::vector<BaseMessagePtr> FormatMessages(const OptionDict& variables);

        PromptValuePtr FormatPrompt() override;

        PromptValuePtr FormatPrompt(const OptionDict& variables) override;

        std::string Format() override;

        std::string Format(const OptionDict& variables) override;
    };
} // core
// langchain

#endif //CHATPROMPTTEMPLATE_H
