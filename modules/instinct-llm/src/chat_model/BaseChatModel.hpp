//
// Created by RobinQu on 2024/1/13.
//

#ifndef BASECHATMODEL_H
#define BASECHATMODEL_H
#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "tools/Assertions.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
;
    class BaseChatModel : public ILanguageModel {
    private:
        virtual BatchedLangaugeModelResult Generate(
            const std::vector<MessageList>& messages
        ) = 0;

        virtual ResultIteratorPtr<LangaugeModelResult> StreamGenerate(const MessageList& messages) = 0;

    public:
        LangaugeModelResult Invoke(const PromptValue& input) override {
            auto messages = convert_prompt_value_messages(input);
            auto batched_result = Generate({messages});
            assert_non_empty_range(batched_result.generations(), "Empty response");
            return batched_result.generations(0);
        }

        ResultIteratorPtr<LangaugeModelResult> Batch(const std::vector<PromptValue>& input) override {
            auto message_matrix = input | std::views::transform(convert_prompt_value_messages);
            auto batched_result = Generate({message_matrix.begin(), message_matrix.end()});
            return create_result_itr_from_range(batched_result.generations());
        }

        ResultIteratorPtr<LangaugeModelResult> Stream(const PromptValue& input) override {
            auto messages = convert_prompt_value_messages(input);
            return StreamGenerate(messages);
        }

    };

    using ChatModelPtr = std::shared_ptr<BaseChatModel>;
}

// core
// langchain

#endif //BASECHATMODEL_H
