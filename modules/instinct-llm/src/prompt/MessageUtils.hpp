//
// Created by RobinQu on 2024/3/10.
//

#ifndef MESSAGEUTILS_HPP
#define MESSAGEUTILS_HPP
#include "LLMGlobals.hpp"
#include "prompt/ChatPromptBuilder.hpp"
#include "tools/StringUtils.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    static MessageRoleNameMapping OLLAMA_ROLE_NAME_MAPPING = {
        {kAsistant, "assistant"},
        {kSystem, "system"},
        {kHuman, "user"},
        {kFunction, "asistant"}
    };

    class MessageUtils {
    public:
        static ChatPromptBuliderPtr CreateOllamaChatPromptBuilder() {
            return std::make_shared<ChatPromptBulider>(OLLAMA_ROLE_NAME_MAPPING);
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
    };
}

#endif //MESSAGEUTILS_HPP
