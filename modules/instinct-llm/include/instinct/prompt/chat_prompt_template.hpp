//
// Created by RobinQu on 2024/1/10.
//

#ifndef CHATPROMPTTEMPLATE_H
#define CHATPROMPTTEMPLATE_H


#include <string>
#include <vector>
#include <instinct/core_global.hpp>
#include <instinct/llm_global.hpp>
#include <instinct/llm.pb.h>
#include <concepts>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class IChatPromptTemplate {
    public:
        IChatPromptTemplate()=default;
        IChatPromptTemplate(const IChatPromptTemplate&)=delete;
        IChatPromptTemplate(IChatPromptTemplate&&)=delete;
        virtual ~IChatPromptTemplate()=default;
        virtual MessageList FormatMessages(const TemplateVariablesPtr& variables) = 0;

        virtual ChatPromptValue FormatChatPrompt(const TemplateVariablesPtr& variables) = 0;

    };
    using ChatPromptTemplatePtr = std::shared_ptr<IChatPromptTemplate>;
    using MessageLikeVariant = std::variant<Message, ChatPromptTemplatePtr>;


    class BaseChatPromptTemplate: public BasePromptTemplate, public virtual IChatPromptTemplate {
    public:
        explicit BaseChatPromptTemplate(const PromptTemplateOptions &options) : BasePromptTemplate(options) {}

        PromptValue FormatPrompt(const TemplateVariablesPtr& variables) override {
            PromptValue prompt_value;
            prompt_value.mutable_chat()->CopyFrom(FormatChatPrompt(variables));
            return prompt_value;
        }

        ChatPromptValue FormatChatPrompt(const TemplateVariablesPtr &variables) override {
            ChatPromptValue cpv;
            auto message_list = FormatMessages(variables);
            for (const auto&msg: message_list.messages()) {
                cpv.add_messages()->MergeFrom(msg);
            }
            return cpv;
        }

        StringPromptValue FormatStringPrompt(const TemplateVariablesPtr& variables) override {
            StringPromptValue spv;
            spv.set_text(Format(variables));
            return spv;
        }

        std::string Format(const TemplateVariablesPtr& variables) override {
            const auto message_list = FormatMessages(variables);
            return MessageUtils::CombineMessages(message_list.messages());
        }

        MessageList FormatMessages(const TemplateVariablesPtr &variables) override = 0;

    };

} // core


#endif //CHATPROMPTTEMPLATE_H
