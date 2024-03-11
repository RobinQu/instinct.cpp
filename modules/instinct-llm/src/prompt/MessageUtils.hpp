//
// Created by RobinQu on 2024/3/10.
//

#ifndef MESSAGEUTILS_HPP
#define MESSAGEUTILS_HPP
#include "LLMGlobals.hpp"
#include "tools/StringUtils.hpp"
#include <fmt/format.h>
#include <fmt/args.h>



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


        static std::string FormatString(const std::string& msg, const ContextPtr& context) {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            // assuming `variables` has depth of one

            for(const auto& [k,v]: context->values()) {
                switch (v.value_case()) {
                    case PrimitiveVariable::kIntValue:
                        store.push_back(fmt::arg(k.c_str(), v.int_value()));
                    break;
                    case PrimitiveVariable::kLongValue:
                        store.push_back(fmt::arg(k.c_str(), v.long_value()));
                    break;
                    case PrimitiveVariable::kFloatValue:
                        store.push_back(fmt::arg(k.c_str(), v.float_value()));
                    break;
                    case PrimitiveVariable::kDoubleValue:
                        store.push_back(fmt::arg(k.c_str(), v.double_value()));
                    break;
                    case PrimitiveVariable::kBoolValue:
                        store.push_back(fmt::arg(k.c_str(), v.bool_value()));
                    break;
                    case PrimitiveVariable::kStringValue:
                        store.push_back(fmt::arg(k.c_str(), v.string_value().c_str()));
                    break;
                    default:
                        throw InstinctException("unknown value type for entry: " + k);
                }
            }
            return fmt::vformat(msg, store);

        }
    };
}

#endif //MESSAGEUTILS_HPP
