//
// Created by RobinQu on 2024/1/12.
//

#ifndef BASELANGUAGEMODEL_H
#define BASELANGUAGEMODEL_H
#include <ranges>
#include <llm.pb.h>
#include "LLMGlobals.hpp"
#include "functional/IRunnable.hpp"
#include "prompt/StringPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    //
    // static auto conv_language_model_input_to_prompt_value = overloaded{
    //     [](const StringPromptValue& v) { return PromptValueVairant{v}; },
    //
    //     [](const ChatPromptValue& v) { return PromptValueVairant(v); },
    //
    //     [](const std::string& v) { return PromptValueVairant{StringPromptValue(v)}; },
    //     [](const MessageVariants& v) {
    //         return PromptValueVairant{ChatPromptValue(v)};
    //     }
    // };

    static std::string convert_prompt_value_to_string(const PromptValue& prompt_value) {

    }

    static MessageList convert_prompt_value_messages(const PromptValue& prompt_value) {

    }

    class ILanguageModel: public IRunnable<PromptValue, LangaugeModelResult> {
    public:
        virtual std::vector<TokenId> GetTokenIds(const std::string& text) = 0;
        virtual TokenSize GetTokenCount(const std::string& text) = 0;
        virtual TokenSize GetTokenCount(const Message& messages) = 0;
    };

    using LanguageModelPtr = std::shared_ptr<ILanguageModel>;

} // core
// langchian

#endif //BASELANGUAGEMODEL_H
