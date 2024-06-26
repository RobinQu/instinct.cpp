//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H

#include <instinct/prompt/IPromptTemplate.hpp>
#include <string>
#include <vector>
#include <instinct/CoreGlobals.hpp>
#include <instinct/LLMGlobals.hpp>
#include <llm.pb.h>
#include <concepts>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class IChatPromptTemplate;
    using ChatPromptTemplatePtr = std::shared_ptr<IChatPromptTemplate>;
    using MessageLikeVariant = std::variant<Message, ChatPromptTemplatePtr>;


    class IChatPromptTemplate {
    public:
        virtual MessageList FormatMessages(const TemplateVariablesPtr& variables) = 0;

        virtual ChatPromptValue FormatChatPrompt(const TemplateVariablesPtr& variables) = 0;

    };

    namespace details {

    }

} // core


#endif //CHATPROMPTTEMPLATE_H
