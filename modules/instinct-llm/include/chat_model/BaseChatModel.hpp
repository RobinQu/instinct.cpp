//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include <utility>

#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "model/ModelCallbackMixins.hpp"
#include "tools/Assertions.hpp"
#include "functional/StepFunctions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class BaseChatModel;
    using ChatModelPtr = std::shared_ptr<BaseChatModel>;

    class ChatModelFunction: public BaseStepFunction {
        ChatModelPtr model_;
    public:
        explicit ChatModelFunction(const ChatModelPtr &model);
        JSONContextPtr Invoke(const JSONContextPtr &input) override;
        AsyncIterator<JSONContextPtr> Batch(const std::vector<JSONContextPtr> &input) override;
        AsyncIterator<JSONContextPtr> Stream(const JSONContextPtr &input) override;
        [[nodiscard]] std::vector<std::string> GetInputKeys() const override;
        [[nodiscard]] std::vector<std::string> GetOutputKeys() const override;
    };


    class BaseChatModel : public virtual ILanguageModel, public BaseRunnable<PromptValueVariant,Message>, public std::enable_shared_from_this<BaseChatModel> {
        friend ChatModelFunction;
        ModelOptions options_;
//        StepFunctionPtr model_function_;
    public:
        explicit BaseChatModel(ModelOptions options) : options_(std::move(options)) {
//            model_function_ = std::make_shared<ChatModelFunction>(shared_from_this());
        }

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

        StepFunctionPtr AsModelfunction() {
            return std::make_shared<ChatModelFunction>(shared_from_this());;
        }

    };

    JSONContextPtr ChatModelFunction::Invoke(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>(model_->options_.input_prompt_variable_key);
        auto messages = details::conv_prompt_value_variant_to_message_list(prompt_value);
        auto batched_result = model_->Generate({messages});

        assert_non_empty_range(batched_result.generations(), "Empty response");
        assert_non_empty_range(batched_result.generations(0).generations(), "Empty response of first output");

        input->PutMessage<Generation>(
                model_->options_.output_answer_variable_key,
                batched_result.generations(0).generations(0)
        );
        return input;
    }

    AsyncIterator<JSONContextPtr> ChatModelFunction::Batch(const std::vector<JSONContextPtr> &input) {
        auto message_matrix_view = input | std::views::transform([&](const auto& ctx) {
            auto prompt_value = ctx->template RequireMessage<PromptValue>(model_->options_.input_prompt_variable_key);
            return details::conv_prompt_value_variant_to_message_list(prompt_value);
        });

        auto batched_results = model_->Generate({message_matrix_view.begin(), message_matrix_view.end()});

        return rpp::source::from_iterable(batched_results.generations())
               | rpp::operators::zip(rpp::source::from_iterable(input))
               | rpp::operators::map([&](const std::tuple<LangaugeModelResult, JSONContextPtr> & tuple) {
            JSONContextPtr output = std::get<1>(tuple);
            LangaugeModelResult mode_result = std::get<0>(tuple);
            output->PutMessage(model_->options_.output_answer_variable_key, mode_result.generations(0));
            return output;
        });
    }

    AsyncIterator<JSONContextPtr> ChatModelFunction::Stream(const JSONContextPtr &input) {
        auto prompt_value = input->RequireMessage<PromptValue>(model_->options_.input_prompt_variable_key);
        auto message_list = details::conv_prompt_value_variant_to_message_list(prompt_value);

        return model_->StreamGenerate(message_list)
               | rpp::operators::map([&](const LangaugeModelResult& result) {
            input->PutMessage(model_->options_.output_answer_variable_key, result.generations(0));
            return input;
        });
    }

    std::vector<std::string> ChatModelFunction::GetInputKeys() const {
        return {model_->options_.input_prompt_variable_key};
    }

    std::vector<std::string> ChatModelFunction::GetOutputKeys() const {
        return {model_->options_.output_answer_variable_key};
    }

    ChatModelFunction::ChatModelFunction(const ChatModelPtr &model) : model_(model) {}


}

// core


#endif //BASECHATMODEL_H
