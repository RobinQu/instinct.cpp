//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H

#include <utility>

#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "tools/Assertions.hpp"
#include "functional/StepFunctions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class BaseChatModel;

    using ChatModelPtr = std::shared_ptr<BaseChatModel>;

    class ChatModelFunction : public BaseStepFunction {
        ChatModelPtr model_;
    public:
        explicit ChatModelFunction(ChatModelPtr model);

        JSONContextPtr Invoke(const JSONContextPtr &input) override;

        AsyncIterator<JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override;

        AsyncIterator<JSONContextPtr> Stream(const JSONContextPtr &input) override;

    };


    class BaseChatModel
            : public virtual ILanguageModel,
              public virtual IConfigurable<ModelOptions>,
              public BaseRunnable<PromptValueVariant, Message>,
              public std::enable_shared_from_this<BaseChatModel> {
        friend ChatModelFunction;
        ModelOptions options_;
        virtual BatchedLangaugeModelResult Generate(
                        const std::vector<MessageList> &messages
                ) = 0;

        virtual AsyncIterator<LangaugeModelResult> StreamGenerate(const MessageList &messages) = 0;
    public:
        explicit BaseChatModel(ModelOptions  options) : options_(std::move(options)) {
//            model_function_ = std::make_shared<ChatModelFunction>(shared_from_this());
        }

        void Configure(const ModelOptions &options) override {
            options_ = options;
        }

        Message Invoke(const PromptValueVariant &input) override {
            auto messages = details::conv_prompt_value_variant_to_message_list(input);
            auto batched_result = Generate({messages});
            assert_non_empty_range(batched_result.generations(), "Empty response");
            return details::conv_language_result_to_message(batched_result.generations(0));
        }

        AsyncIterator<Message> Batch(const std::vector<PromptValueVariant> &input) override {
            auto message_matrix = input | std::views::transform(details::conv_prompt_value_variant_to_message_list);
            auto batched_result = Generate({message_matrix.begin(), message_matrix.end()});
            return rpp::source::from_iterable(batched_result.generations())
                   | rpp::operators::map(details::conv_language_result_to_message);
        }

        AsyncIterator<Message> Stream(const PromptValueVariant &input) override {
            auto messages = details::conv_prompt_value_variant_to_message_list(input);
            return StreamGenerate(messages)
                   | rpp::operators::map(details::conv_language_result_to_message);
        }

        StepFunctionPtr AsModelFunction() {
            return std::make_shared<ChatModelFunction>(shared_from_this());;
        }

        void BindToolSchemas(const std::vector<FunctionToolSchema> &function_tool_schema) override {
            throw InstinctException("Not implemented");
        }

    };

    inline JSONContextPtr ChatModelFunction::Invoke(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>();
        auto messages = details::conv_prompt_value_variant_to_message_list(prompt_value);
        auto batched_result = model_->Generate({messages});

        assert_non_empty_range(batched_result.generations(), "Empty response");
        assert_non_empty_range(batched_result.generations(0).generations(), "Empty response of first output");

        input->ProduceMessage<Generation>(
                batched_result.generations(0).generations(0)
        );
        return input;
    }

    inline AsyncIterator<JSONContextPtr> ChatModelFunction::Batch(const std::vector<JSONContextPtr> &input) {
        auto message_matrix_view = input | std::views::transform([&](const auto &ctx) {
            auto prompt_value = ctx->template RequireMessage<PromptValue>();
            return details::conv_prompt_value_variant_to_message_list(prompt_value);
        });

        auto batched_results = model_->Generate({message_matrix_view.begin(), message_matrix_view.end()});

        return rpp::source::from_iterable(batched_results.generations())
               | rpp::operators::zip(rpp::source::from_iterable(input))
               | rpp::operators::map([&](const std::tuple<LangaugeModelResult, JSONContextPtr> &tuple) {
            JSONContextPtr output = std::get<1>(tuple);
            LangaugeModelResult mode_result = std::get<0>(tuple);
            output->ProduceMessage(mode_result.generations(0));
            return output;
        });
    }

    inline AsyncIterator<JSONContextPtr> ChatModelFunction::Stream(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>();
        auto message_list = details::conv_prompt_value_variant_to_message_list(prompt_value);

        return model_->StreamGenerate(message_list)
               | rpp::operators::map([&](const LangaugeModelResult &result) {
            input->ProduceMessage(result.generations(0));
            return input;
        });
    }


    inline ChatModelFunction::ChatModelFunction(ChatModelPtr model) : model_(std::move(model)) {}


}

// core


#endif //BASECHATMODEL_H
