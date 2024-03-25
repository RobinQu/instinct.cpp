//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASECHATPROMPTTEMPLATE_HPP
#define INSTINCT_BASECHATPROMPTTEMPLATE_HPP
#include "LLMGlobals.hpp"
#include "BasePromptTemplate.hpp"
#include "IChatPromptTemplate.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

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
}

#endif //INSTINCT_BASECHATPROMPTTEMPLATE_HPP
