//
// Created by RobinQu on 2024/3/7.
//

#ifndef LLMCHAIN_HPP
#define LLMCHAIN_HPP

#include <llm.pb.h>

#include "LLMGlobals.hpp"
#include "chain//IChain.hpp"
#include "model/ILanguageModel.hpp"
#include "../llm/BaseLLM.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "output_parser/IOutputParser.hpp"
#include "prompt/IPromptTemplate.hpp"
#include "memory/IChatMemory.hpp"
#include "tools/Assertions.hpp"

namespace
INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    // to adapt both LLM and ChatModel
    using LanguageModelVariant = std::variant<LLMPtr, ChatModelPtr>;

    template<typename Result>
    class LLMChain : public IChain<Result> {
        LanguageModelVariant model_{};
        PromptTemplatePtr prompt_template_{};
        OutputParserPtr<Result> output_parser_{};
        IChatMemoryPtr chat_memory_{};

    public:
        LLMChain(const LanguageModelVariant& model,
                 const PromptTemplatePtr& prompt_template,
                 const OutputParserPtr<Result>& output_parser,
                 const IChatMemoryPtr& chat_memory)
            : model_(model),
              prompt_template_(prompt_template),
              output_parser_(output_parser),
              chat_memory_(chat_memory) {
        }

        Result Invoke(const LLMChainContext& input) override {
            auto prompt_value = prompt_template_->FormatPrompt(input);
            if (std::holds_alternative<LLMPtr>(model_)) {
                auto llm = std::get<LLMPtr>(model_);
                return output_parser_->ParseResult(llm->Invoke(prompt_value));
            }
            if (std::holds_alternative<ChatModelPtr>(model_)) {
                auto chat_model = std::get<ChatModelPtr>(model_);
                return output_parser_->ParseResult(chat_model->Invoke(prompt_value).content());
            }
            throw InstinctException("invalid model pointer");
        }

        ResultIteratorPtr<Result> Batch(const std::vector<LLMChainContext>& input) override {
            auto prompt_value_view = input | std::views::transform([&](auto&& ctx) {
                return prompt_template_->FormatPrompt(ctx);
            });

            if (std::holds_alternative<LLMPtr>(model_)) {
                auto llm = std::get<LLMPtr>(model_);
                auto result_itr = llm->Batch({prompt_value_view.begin(), prompt_value_view.end()});
                return create_result_itr_with_transform([&](auto&& model_result) {
                    return output_parser_->ParseResult(model_result);
                }, result_itr);
            }

            if (std::holds_alternative<ChatModelPtr>(model_)) {
                auto chat_model = std::get<ChatModelPtr>(model_);
                auto result_itr = chat_model->Batch({prompt_value_view.begin(), prompt_value_view.end()});
                return create_result_itr_with_transform( [&](auto&& mesage) {
                    return output_parser_->ParseResult(mesage.content());
                }, result_itr);
            }

            throw InstinctException("invalid model pointer");
        }

        ResultIteratorPtr<Result> Stream(const LLMChainContext& input) override {
            auto prompt_value = prompt_template_->FormatPrompt(input);

            if (std::holds_alternative<LLMPtr>(model_)) {
                auto llm = std::get<LLMPtr>(model_);
                auto chunk_itr = llm->Stream(prompt_value);
                return create_result_itr_with_transform([&](auto&& model_result) {
                    return output_parser_->ParseResult(model_result);
                }, chunk_itr);
            }

            if (std::holds_alternative<ChatModelPtr>(model_)) {
                auto chat_model = std::get<ChatModelPtr>(model_);
                auto chunk_itr = chat_model->Stream(prompt_value);
                return create_result_itr_with_transform([&](auto&& message) {
                   return message.content();
                }, chunk_itr);
            }

            throw InstinctException("invalid model pointer");
        }
    };


    // IRunnable<Input,Output>

    // ILangaugeModel<Input,Output> extends IRunnable<Input,Output>
    // ILLM<LLMInputVariant, std::string>
    // ChatModel<ChatModelInputVariant, Message>

    // IChain<ResultMessage> extends IRunnable<LLMChainContext, ResultMessage>
    // LLMChain<T> implments IChain<T>

    // # LLMChain<T>
    // LLMChainContext -> (prompt_template.Format) -> StringPromptValue
    // StringPromptValue -> (LM.Invoke) -> LangaugeModelOutput
    // LangaugeModelOutput -> (OutputParser.Invoke) -> T

    // std::vector<FormattableVariables> -> (prompt_template.Batch) -> ResultItr<StringPromptValue>*
    // ResultItr<StringPromptValue>* -> BatchedLanguageModelInput -> (LM.Batch) -> BatchedLangaugeModelOutput
    // BatchedLangaugeModelOutput -> (OutputParser.Batch) -> ResultItr<T>*

    // LLMChainContext -> (prompt_template.Invoke) -> StringPromptValue
    // StringPromptValue -> (LM.Stream) -> ResultItr<LangaugeModelOutput>*
    // ResultItr<LangaugeModelOutput>* -> loop with (OutputParser.Invoke) -> ResultItr<LLMChainResponse>*


    // # VallilanRAGChain<T>
    // LLMChainContext -> (DocumentRetriever.Retreive) -> ResultItr<Document>*;
    // ResultItr<Document>* combined to LLMChainContext['context']
    // LLMChainContext -> (prompt_template.Invoke) -> StringPromptValue
    // StringPromptValue -> (LM.Invoke) -> LangaugeModelOutput
    // LangaugeModelOutput -> (OutputParser.Invoke) -> T

    // ConvsersationalRAGChain<T> , with QuestionChain<PrimitiveVariable> as q_c, VallilanRAGChain<AnswerMessage> as vallilan_rag
    // LLMChainContext -> (ChatMemory.Invoke) -> ChatMessageHistory
    // combine ChatMessageHistory to string, and set into LLMChainContext['chat_history']
    // LLMChainContext -> (q_c.Invoke) -> std::string
    // merged returned variable to LLMChainContext
    // LLMChainContext -> (vallilan_rag.Invoke) -> AnswerMessage
}


#endif //LLMCHAIN_HPP
