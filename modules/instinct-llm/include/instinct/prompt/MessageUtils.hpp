//
// Created by RobinQu on 2024/3/10.
//

#ifndef MESSAGEUTILS_HPP
#define MESSAGEUTILS_HPP

#include <fmt/format.h>
#include <fmt/args.h>

#include <instinct/LLMGlobals.hpp>
#include <instinct/tools/string_utils.hpp>


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    class MessageUtils {
    public:

        static PromptValue ConvertPromptValueVariantToPromptValue(const PromptValueVariant& pvv) {
            return std::visit(overloaded {
                [](const StringPromptValue& spv) { PromptValue pv; pv.mutable_string()->CopyFrom(spv); return pv; },
            [](const ChatPromptValue& cpv) { PromptValue pv; pv.mutable_chat()->CopyFrom(cpv);  return pv; },
            [](const PromptValue& pv) { return pv; },
            [](const MessageList& message_list) { PromptValue pv;  pv.mutable_chat()->mutable_messages()->Add(message_list.messages().begin(), message_list.messages().end()); return pv; },
            [](const Message& message) { PromptValue pv;  pv.mutable_chat()->mutable_messages()->Add()->CopyFrom(message); return pv; },
            [](const std::string& str) { PromptValue pv; pv.mutable_string()->set_text(str); return pv;}
            }, pvv);
        }


        static std::string ExtractLatestPromptString(const PromptValue& pv) {
            if (pv.has_chat()) {
                // ignore history messages
                if (const auto msg_count = pv.chat().messages_size(); msg_count > 0) {
                    return pv.chat().messages(msg_count-1).content();
                }
            }
            if (pv.has_string()) {
                return pv.string().text();
            }
            return "";
        }

        static std::string StringifyGeneration(const Generation &generation) {
            if (generation.has_message()) return generation.message().content();
            return generation.text();
        }

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

            const auto result = fmt::vformat(msg, store);
            LOG_DEBUG("FormatPrompt {}, total char8_t string length: {}", msg.substr(0, 100), result.size());
            return result;
        }
    };
}

#endif //MESSAGEUTILS_HPP
