//
// Created by RobinQu on 2024/2/13.
//

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H
#include "CoreGlobals.h"
#include "StringPromptTemplate.h"

namespace
LC_CORE_NS {
    class PromptTemplate;
    using PromptTemplatePtr = std::shared_ptr<PromptTemplate>;

    class PromptTemplate : public StringPromptTemplate {
        std::string template_string_;

    public:
        static PromptTemplatePtr FromTemplate(const std::string& template_string);

        explicit PromptTemplate(std::string template_string)
            : StringPromptTemplate(), template_string_(std::move(template_string)) {
        }

        std::string Format() override;

        std::string Format(const OptionDict& variables) override;
    };
} // LC_CORE_NS

#endif //PROMPTTEMPLATE_H
