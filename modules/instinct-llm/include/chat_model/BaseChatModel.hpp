//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "model/ModelCallbackMixins.hpp"
#include "tools/Assertions.hpp"
#include "functional/StepFunctions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
;
    class BaseChatModel : public virtual ILanguageModel, public BaseStepFunction, public BaseRunnable<PromptValueVariant,Message> {
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

        [[nodiscard]] std::vector<std::string> GetInputKeys() const override {
            return {DEFAULT_MESSAGE_LIST_INPUT_KEY};
        }

        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override {
            return {DEFAULT_ANSWER_OUTPUT_KEY};
        }

        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            // TODO fetch MessageList from input
            auto message_list = input->RequireMessage<MessageList>(DEFAULT_MESSAGE_LIST_INPUT_KEY);
            auto result = Invoke(message_list);
        }

        AsyncIterator<instinct::core::JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override {
            auto message_matrix_view = input | std::views::transform([](const auto& ctx) {
                auto message_list = ctx->template RequireMessage<MessageList>(DEFAULT_MESSAGE_LIST_INPUT_KEY);
                return PromptValueVariant  {message_list};
            });

            const std::vector<PromptValueVariant> message_matrix {message_matrix_view.begin(), message_matrix_view.end()};

            return Batch(message_matrix)
                | rpp::operators::map([&](const auto& message) {
                    auto ctx = CreateJSONContext();
                    ctx->PutMessage(DEFAULT_MESSAGE_OUTPUT_KEY, message);
                    return ctx;
                });
        }

        AsyncIterator<instinct::core::JSONContextPtr> Stream(const JSONContextPtr &input) override {
            auto message_list = input->RequireMessage<MessageList>(DEFAULT_MESSAGE_LIST_INPUT_KEY);
            return Stream(message_list)
                | rpp::operators::map([&](const auto& message) {
                    auto ctx = CreateJSONContext();
                    ctx->PutMessage(DEFAULT_MESSAGE_OUTPUT_KEY, message);
                    return ctx;
                });
        }
    };

    using ChatModelPtr = std::shared_ptr<BaseChatModel>;
}

// core


#endif //BASECHATMODEL_H
