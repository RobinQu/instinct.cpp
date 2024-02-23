//
// Created by RobinQu on 2024/2/13.
//

#ifndef PROMPTTEMPLATE_H
#define PROMPTTEMPLATE_H
#include "CoreGlobals.hpp"
#include <fmt/format.h>
#include <fmt/args.h>

#include "StringPromptTemplate.hpp"


LC_CORE_NS {

    class PlainPromptTemplate : public StringPromptTemplate {
        std::string template_string_;

    public:
        static PlainPromptTemplate FromTemplate(const std::string& template_string);

        explicit PlainPromptTemplate(std::string template_string)
            : StringPromptTemplate(), template_string_(std::move(template_string)) {
        }
        std::string Format(const OptionDict& variables) override;
    };

    inline PlainPromptTemplate PlainPromptTemplate::FromTemplate(const std::string& template_string) {
        return PlainPromptTemplate(template_string);
    }

    inline std::string PlainPromptTemplate::Format(const OptionDict& variables) {
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        // assuming `variables` has depth of one
        for(auto itr=variables.begin(); itr!=variables.end(); ++itr) {
            store.push_back(fmt::arg(itr.key().c_str(), itr.value().dump()));
        }
        return fmt::vformat(this->template_string_, store);
    }

} // LC_CORE_NS

#endif //PROMPTTEMPLATE_H
