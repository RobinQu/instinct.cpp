//
// Created by RobinQu on 2024/2/13.
//

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H


#include <instinct/llm_global.hpp>

#include <instinct/prompt/string_prompt_template.hpp>
#include <instinct/prompt/message_utils.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class PlainPromptTemplate final: public StringPromptTemplate {
        std::string template_string_;

    public:
        PlainPromptTemplate(std::string templateString, const PromptTemplateOptions &options)
                : StringPromptTemplate(options), template_string_(std::move(templateString)) {}

        std::string Format(const TemplateVariablesPtr & variables) override {
            return MessageUtils::FormatString(this->template_string_, variables);
        }
    };

    static PromptTemplatePtr CreatePlainPromptTemplate(const std::string& template_string, const PromptTemplateOptions &options = {}) {
        return std::make_shared<PlainPromptTemplate>(template_string, options);
    }


} // namespace INSTINCT_CORE_NS

#endif //PROMPTTEMPLATE_H
