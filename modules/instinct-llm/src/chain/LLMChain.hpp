//
// Created by RobinQu on 2024/3/7.
//

#ifndef LLMCHAIN_HPP
#define LLMCHAIN_HPP

#include <llm.pb.h>

#include "BaseChain.hpp"
#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "llm/BaseLLM.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/IPromptTemplate.hpp"
#include "memory/BaseChatMemory.hpp"
#include "tools/Assertions.hpp"

namespace
INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    // to adapt both LLM and ChatModel
    using LanguageModelVariant = std::variant<LLMPtr, ChatModelPtr>;

    template<typename Result>
    class LLMChain final: public BaseChain<Result> {
        LanguageModelVariant model_{};
        PromptTemplatePtr prompt_template_{};
        OutputParserPtr<Result> output_parser_{};
        ChatMemoryPtr chat_memory_{};
    public:
        LLMChain(const LanguageModelVariant& model,
                 const PromptTemplatePtr& prompt_template,
                 const OutputParserPtr<Result>& output_parser,
                 const ChatMemoryPtr& chat_memory,
                 ChainOptions options = {}
                 )
            : BaseChain<Result>(std::move(options)),
                model_(model),
              prompt_template_(prompt_template),
              output_parser_(output_parser),
              chat_memory_(chat_memory) {

        }

        void EnhanceContext(const ChainContextBuilderPtr& context_builder) override {
            if (chat_memory_) {
                // add chat histroy
                chat_memory_->EnhanceContext(context_builder);
            }

            // add parser instruction
            output_parser_->EnhanceContext(context_builder);
        }


        Result Invoke(const LLMChainContext& input) override {
            auto context_builder = ChainContextBuilder::Create(input);
            EnhanceContext(context_builder);

            auto prompt_value = prompt_template_->FormatPrompt(context_builder->Build());
            if (std::holds_alternative<LLMPtr>(model_)) {
                const auto llm = std::get<LLMPtr>(model_);
                auto text = llm->Invoke(prompt_value);

                context_builder->Put(this->GetOptions().output_answer_content_key, text);
                // TODO Role name normalization
                context_builder->Put(this->GetOptions().output_ansewr_role_key, "human");

                Generation generation;
                generation.set_text(text);
                if (chat_memory_) {
                    chat_memory_->SaveMemory(context_builder->Build());
                }
                return output_parser_->ParseResult(generation);
            }
            if (std::holds_alternative<ChatModelPtr>(model_)) {
                const auto chat_model = std::get<ChatModelPtr>(model_);
                Generation generation;
                const auto message = chat_model->Invoke(prompt_value);
                generation.mutable_message()->CopyFrom(message);
                context_builder->Put(this->GetOptions().output_ansewr_role_key, message.role());
                context_builder->Put(this->GetOptions().output_answer_content_key, message.content());
                if (chat_memory_) {
                    chat_memory_->SaveMemory(context_builder->Build());
                }
                return output_parser_->ParseResult();
            }
            throw InstinctException("invalid model pointer");
        }


    };

    template<typename T>
    using LLMChainPtr = std::shared_ptr<LLMChain<T>>;


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
