//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <string>

#include "functional/JSONContextPolicy.hpp"
#include "LLMGlobals.hpp"


namespace INSTINCT_LLM_NS {
    class IPromptTemplate {
    public:
        virtual ~IPromptTemplate() = default;

        virtual PromptValue FormatPrompt(const JSONContextPtr& variables) = 0;

        virtual std::string Format(const JSONContextPtr& variables) = 0;

        virtual StringPromptValue FormatStringPrompt(const JSONContextPtr& variables) = 0;

    };

    using PromptTemplatePtr = std::shared_ptr<IPromptTemplate>;

}


#endif //PROMPT_H
