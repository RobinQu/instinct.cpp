//
// Created by RobinQu on 2024/2/13.
//

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H
#include "LLMGlobals.hpp"

#include "StringPromptTemplate.hpp"
#include "prompt/MessageUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class PlainPromptTemplate : public StringPromptTemplate {
        std::string template_string_;

    public:
        static PromptTemplatePtr CreateWithTemplate(const std::string& template_string) {
            return std::make_shared<PlainPromptTemplate>(template_string);
        }

        explicit PlainPromptTemplate(std::string template_string)
            : StringPromptTemplate(), template_string_(std::move(template_string)) {
        }

        std::string Format(const ContextPtr& variables) override {
            return MessageUtils::FormatString(this->template_string_, variables);
        }
    };


} // namespace INSTINCT_CORE_NS

#endif //PROMPTTEMPLATE_H
