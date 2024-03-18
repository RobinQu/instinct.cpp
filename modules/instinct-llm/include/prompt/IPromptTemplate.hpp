//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <string>


#include "LLMGlobals.hpp"


namespace INSTINCT_LLM_NS {
    class IPromptTemplate {
    public:
        virtual ~IPromptTemplate() = default;

        virtual PromptValue FormatPrompt(const ContextPtr& variables) = 0;

        virtual std::string Format(const ContextPtr& variables) = 0;

        virtual StringPromptValue FormatStringPrompt(const ContextPtr& variables) = 0;

    };

    using PromptTemplatePtr = std::shared_ptr<IPromptTemplate>;



    // IPromptTemplate, Format() -> std::string, FormatPrompt -> StringPromptValue
    // StringPrompteTemplate implements IPromptTemplate
    // FewShotPromptTemplate implements IPromptTemplate


    // PromptTemplatePtr = std::shared<IPromptTemplate>
    // MessageALike = std::variant<Message, PromptTemplatePtr>

    // IChatPromptTemplate extends IPromptTemplate

    // IChatPromptTemplate, FormatMessages -> [Message], FormatPrompt -> ChatPromptValue
    //
    // FewShotChatPromptTemplate
    // MessagePromptTemplate extends ChatPromptTemplate

} // model


#endif //PROMPT_H
