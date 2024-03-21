//
// Created by RobinQu on 2024/1/9.
//

#ifndef STRINGPROMPTTEMPLATE_H
#define STRINGPROMPTTEMPLATE_H


#include <unicode/format.h>

#include "BasePromptTemplate.hpp"
#include "LLMGlobals.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class StringPromptTemplate : public BasePromptTemplate {
    public:
        explicit StringPromptTemplate(const PromptTemplateOptions &options) : BasePromptTemplate(options) {}

        PromptValue FormatPrompt(const JSONContextPtr& variables) override {
            PromptValue pv;
            pv.mutable_string()->CopyFrom(FormatStringPrompt(variables));
            return pv;
        }

        std::string Format(const JSONContextPtr& variables) override = 0;

        StringPromptValue FormatStringPrompt(const JSONContextPtr& variables) override {
            StringPromptValue spv;
            spv.set_text(Format(variables));
            return spv;
        }


    };


} // core


#endif //STRINGPROMPTTEMPLATE_H
