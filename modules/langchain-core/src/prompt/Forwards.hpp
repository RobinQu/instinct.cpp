//
// Created by RobinQu on 2024/2/22.
//

#ifndef FORWARDS_HPP
#define FORWARDS_HPP
#include "CoreGlobals.hpp"
#include <variant>


LC_CORE_NS {
    class StringPromptValue;
    class AIMessage;
    class HumanMessage;
    class FunctionMessage;
    class SystemMessage;
    class ChatMessage;
    class StringPromptValue;
    class ChatPromptValue;
    struct Generation;
    struct ChatGeneration;
    class HumanMessagePromptTemplate;
    class SystemMessagePromptTemplate;
    class ChatMessagePromptTemplate;
    class FewshotChatMessagePromptTemplate;
    class AIMessagePromptTemplate;


    using PromptValueVairant = std::variant<StringPromptValue, ChatPromptValue>;
    using PromptValueVairants = std::vector<PromptValueVairant>;
    using MessageVariant = std::variant<AIMessage, HumanMessage, FunctionMessage, SystemMessage, ChatMessage>;
    using MessageLikeVariant = std::variant<
        AIMessage, HumanMessage, FunctionMessage, SystemMessage, ChatMessage,
        HumanMessagePromptTemplate, SystemMessagePromptTemplate, ChatMessagePromptTemplate,
        FewshotChatMessagePromptTemplate, AIMessagePromptTemplate
    >;
    using MessageVariants = std::vector<MessageVariant>;
    using LanguageModelInput = std::variant<StringPromptValue, ChatPromptValue, std::string, MessageVariants>;
    using GenerationVariant = std::variant<Generation, ChatGeneration>;
    using TokenSize = unsigned long;
    using TokenId = unsigned long;
}


#endif //FORWARDS_HPP
