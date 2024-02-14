//
// Created by RobinQu on 2024/2/13.
//

#include "PromptTemplate.h"
#include <fmt/format.h>
#include <fmt/args.h>

namespace LC_CORE_NS {


    PromptTemplatePtr PromptTemplate::FromTemplate(const std::string& template_string) {
        return std::make_shared<PromptTemplate>(template_string);
    }

    std::string PromptTemplate::Format(const OptionDict& variables) {
        fmt::dynamic_format_arg_store<fmt::format_context> store;
        // assuming `variables` has depth of one
        for(auto itr=variables.begin(); itr!=variables.end(); ++itr) {
            store.push_back(fmt::arg(itr.key().c_str(), itr.value().dump()));
        }
        return fmt::vformat(this->template_string_, store);
    }

    std::string PromptTemplate::Format() {
        return Format({});
    }
} // LC_CORE_NS