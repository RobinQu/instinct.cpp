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
#include "output_parser/IOutputParser.hpp"
#include "prompt/IPromptTemplate.hpp"
#include "memory/ChatMemory.hpp"
#include "tools/Assertions.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;


    template<typename Result>
    class LLMChain: public IChain<Result> {
        LanguageModelPtr model_{};
        PromptTemplatePtr prompt_template_{};
        OutputParserPtr<Result> output_parser_{};
        IChatMemoryPtr chat_memory_{};

    public:
        LLMChain(LLMPtr model, PromptTemplatePtr prompt_template, OutputParserPtr<Result> output_parser,
            IChatMemoryPtr chat_memory)
            : model_(std::move(model)),
              prompt_template_(std::move(prompt_template)),
              output_parser_(std::move(output_parser)),
              chat_memory_(std::move(chat_memory)) {
        }

        Result Invoke(const LLMChainContext& input) override {
            auto prompt_value = prompt_template_->FormatPrompt(input);
            auto model_result = model_->Invoke(prompt_value);
            auto output = model_result.generations(0);
            return output_parser_->ParseResult(output);
        }

        ResultIteratorPtr<Result> Batch(const std::vector<LLMChainContext>& input) override {
            auto prompt_value_view = input | std::views::transform([&](auto&& ctx) {
                return prompt_template_->FormatPrompt(std::forward<LLMChainContext>(ctx));
            });
            auto result_itr = model_->Batch({prompt_value_view.begin(), prompt_value_view.end()});

            return create_result_itr_with_transform(result_itr, [&](auto&& model_result) {
                return output_parser_->ParseResult(model_result.generations[0]);
            });
        }

        ResultIteratorPtr<Result> Stream(const LLMChainContext& input) override {
            auto prompt_value = prompt_template_->FormatPrompt(input);
            auto chunk_itr = model_->Stream(prompt_value);
            return create_result_itr_with_transform(chunk_itr, [&](auto&& model_result) {
                return output_parser_->ParseResult(model_result.generations[0]);
            });
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
