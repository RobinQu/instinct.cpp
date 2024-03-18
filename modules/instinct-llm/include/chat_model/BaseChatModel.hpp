//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "model/ModelCallbackMixins.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
;
    class BaseChatModel : public ILanguageModel<Message>, public ChatModelCallbackMixins{
    private:
        virtual BatchedLangaugeModelResult Generate(
            const std::vector<MessageList>& messages
        ) = 0;

        virtual AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList& messages) = 0;

    public:
        Message Invoke(const PromptValueVariant& input) override {
            auto messages = details::conv_prompt_value_variant_to_message_list(input);
            auto batched_result = Generate({messages});
            assert_non_empty_range(batched_result.generations(), "Empty response");
            return details::conv_language_result_to_message(batched_result.generations(0));
        }

        AsyncIterator<Message> Batch(const std::vector<PromptValueVariant>& input) override {
            auto message_matrix = input | std::views::transform(details::conv_prompt_value_variant_to_message_list);
            auto batched_result = Generate({message_matrix.begin(), message_matrix.end()});
            return rpp::source::from_iterable(batched_result.generations())
                | rpp::operators::map(details::conv_language_result_to_message);
        }

        AsyncIterator<Message> Stream(const PromptValueVariant& input) override {
            auto messages = details::conv_prompt_value_variant_to_message_list(input);
            return StreamGenerate(messages)
                | rpp::operators::map(details::conv_language_result_to_message);
        }

    };

    using ChatModelPtr = std::shared_ptr<BaseChatModel>;
}

// core
// langchain

#endif //BASECHATMODEL_H
