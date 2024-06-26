//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <string>

#include <instinct/functional/json_context.hpp>
#include <instinct/LLMGlobals.hpp>


namespace INSTINCT_LLM_NS {



    class IPromptTemplate {
    public:
        virtual ~IPromptTemplate() = default;

        virtual PromptValue FormatPrompt(const TemplateVariablesPtr& variables) = 0;

        virtual std::string Format(const TemplateVariablesPtr& variables) = 0;

        virtual StringPromptValue FormatStringPrompt(const TemplateVariablesPtr& variables) = 0;

    };



}


#endif //PROMPT_H
