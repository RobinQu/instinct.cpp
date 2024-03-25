//
// Created by RobinQu on 2024/3/10.
//

#ifndef MESSAGEUTILS_HPP
#define MESSAGEUTILS_HPP

#include <fmt/format.h>
#include <fmt/args.h>


#include "functional/JSONContextPolicy.hpp"
#include "LLMGlobals.hpp"
#include "tools/StringUtils.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    class MessageUtils {
    public:

        static PromptValue CreateStringPrompt(const std::string& s) {
            PromptValue pv;
            pv.mutable_string()->set_text(s);
            return pv;
        }

        template<typename R>
        requires RangeOf<R, Message>
        static std::string CombineMessages(R&& message_list) {
            auto content_view = message_list | std::views::transform([](auto&& m) {
                return fmt::format("{}: {}", m.role(), m.content());
            });
            return StringUtils::JoinWith(content_view, "\n");
        }


        static std::string FormatMessage(
            const Message& message,
            const std::string& template_string = "{role}: {content}") {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(fmt::arg("role", message.role()));
            store.push_back(fmt::arg("content", message.content()));
            return fmt::vformat(template_string, store);
        }


        static std::string FormatString(
                const std::string& msg,
                const TemplateVariablesPtr & context) {
            LOG_DEBUG("FormatPrompt {} with variables: {}", msg, context->dump());
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            // assuming `context` has depth of one
            for(const auto& [k,v]: context->items()) {
                if (v.is_number_integer()) {
                    store.push_back(fmt::arg(k.c_str(), v.get<long>()));
                }
                if(v.is_number_float()) {
                    store.push_back(fmt::arg(k.c_str(), v.get<double>()));
                }
                if(v.is_string()) {
                    store.push_back(fmt::arg(k.c_str(), v.get<std::string>().c_str()));
                }
                if(v.is_boolean()) {
                    store.push_back(fmt::arg(k.c_str(), v.get<bool>()));
                }
            }
            return fmt::vformat(msg, store);
        }
    };
}

#endif //MESSAGEUTILS_HPP
