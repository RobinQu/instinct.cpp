//
// Created by RobinQu on 2024/1/9.
//

#ifndef PROMPT_H
#define PROMPT_H

#include <any>
#include <string>

#include "CoreTypes.hpp"
#include "Forwards.hpp"
#include "CoreGlobals.hpp"


LC_CORE_NS {
    class PromptTemplate {
    public:
        virtual ~PromptTemplate() = default;

        virtual PromptValueVairant FormatPrompt() {
            return FormatPrompt({});
        }

        virtual PromptValueVairant FormatPrompt(const OptionDict& variables) = 0;

        virtual std::string Format() {
            return Format({});
        }

        virtual std::string Format(const OptionDict& variables) = 0;

        // virtual std::string FormatDocument(DocumentPtr document) = 0;
        // virtual
        //
        // void Save(std::filesystem::path filepath) = 0;
    };
} // model
// langchain

#endif //PROMPT_H
