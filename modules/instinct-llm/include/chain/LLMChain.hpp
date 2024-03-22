//
// Created by RobinQu on 2024/3/7.
//

#ifndef LLMCHAIN_HPP
#define LLMCHAIN_HPP

#include <llm.pb.h>

#include <utility>

#include "MessageChain.hpp"
#include "LLMGlobals.hpp"
#include "model/ILanguageModel.hpp"
#include "llm/BaseLLM.hpp"
#include "chat_model/BaseChatModel.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "prompt/IPromptTemplate.hpp"
#include "memory/BaseChatMemory.hpp"
#include "tools/Assertions.hpp"
#include "input_parser/PromptValueInputParser.hpp"
#include "output_parser/GenerationOutputParser.hpp"
#include "output_parser/MultilineGenerationOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    // to adapt both LLM and ChatModel
    using LanguageModelVariant = std::variant<LLMPtr, ChatModelPtr>;






    template<typename Output>
    class LLMChain final : public MessageChain<Output> {
        LanguageModelVariant model_{};
        PromptTemplatePtr prompt_template_{};
        ChatMemoryPtr chat_memory_{};

    public:

        LLMChain(PromptTemplatePtr promptTemplate,
                 LanguageModelVariant model,
                 const OutputParserPtr<Output> &outputConverter,
                 ChatMemoryPtr chatMemory,
                 const ChainOptions& options) :
                MessageChain<Output>(outputConverter, options),
                model_(std::move(model)),
                prompt_template_(std::move(promptTemplate)),
                chat_memory_(std::move(chatMemory)) {}

        StepFunctionPtr GetStepFunction() override {
            StepFunctionPtr model_function;
            if (std::holds_alternative<LLMPtr>(model_)) {
                model_function = std::get<LLMPtr>(model_)->AsModelFunction();
            }
            if (std::holds_alternative<ChatModelPtr>(model_)) {
                model_function = std::get<ChatModelPtr>(model_)->AsModelfunction();
            }
            assert_true(model_function, "should contain model function");

            if (chat_memory_) {
                return this->output_converter_->AsInstructorFunction()
                          | chat_memory_->AsLoadMemoryFunction()
                          | prompt_template_
                          | model_function
                          | chat_memory_->AsSaveMemoryFunction();
            }
            return this->output_converter_->AsInstructorFunction()
                   | prompt_template_
                   | model_function;
        }

    };


    using TextChain = LLMChain<Generation>;
    using TextChainPtr = std::shared_ptr<TextChain>;
    using MultilineChain = LLMChain<MultilineGeneration>;
    using MultilineChainPtr = std::shared_ptr<MultilineChain>;

    using StructuredChain = LLMChain<StructuredGeneration>;
    using StructuredChainPtr = std::shared_ptr<StructuredChain>;


    static MultilineChainPtr CreateMultilineChain(
            const LanguageModelVariant &model,
            const PromptTemplatePtr& prompt_template = nullptr,
            const ChainOptions& options = {},
            const ChatMemoryPtr& chat_memory = nullptr,
            OutputParserPtr<MultilineGeneration> output_parser = nullptr
            ) {
        if (!output_parser) {
            output_parser = std::make_shared<MultilineGenerationOutputParse>();
        }
        return std::make_shared<MultilineChain>(
                prompt_template,
                model,
                output_parser,
                chat_memory,
                options
        );
    }


    static TextChainPtr CreateTextChain(
            const LanguageModelVariant &model,
            const PromptTemplatePtr& prompt_template = nullptr,
            const ChainOptions& options = {},
            const ChatMemoryPtr& chat_memory = nullptr,
            OutputParserPtr<Generation> output_parser = nullptr
    ) {
        if (!output_parser) {
            output_parser = std::make_shared<GenerationOutputParser>();
        }
        return std::make_shared<TextChain>(
                prompt_template,
                model,
                output_parser,
                chat_memory,
                options
        );
    }


}


#endif //LLMCHAIN_HPP
