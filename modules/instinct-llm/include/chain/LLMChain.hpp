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

    template<typename Input, typename Output>
    class LLMChain;

    using TextChain = LLMChain<PromptValue, Generation>;
    using TextChainPtr = std::shared_ptr<TextChain>;
    using MultilineChain = LLMChain<PromptValue,MultilineGeneration>;
    using MultilineChainPtr = std::shared_ptr<MultilineChain>;

    using StructuredChain = LLMChain<PromptValue, StructuredGeneration>;
    using StructuredChainPtr = std::shared_ptr<StructuredChain>;




    template<typename Input, typename Output>
    class LLMChain final : public MessageChain<Input, Output> {
        LanguageModelVariant model_{};
        PromptTemplatePtr prompt_template_{};
        ChatMemoryPtr chat_memory_{};

    public:

        LLMChain(const InputParserPtr<Input> &inputConverter,
                 const OutputParserPtr<Output> &outputConverter,
                 LanguageModelVariant model,
                 PromptTemplatePtr promptTemplate,
                 ChatMemoryPtr chatMemory,
                 const ChainOptions& options) :
                MessageChain<Input, Output>(inputConverter, outputConverter, options),
                model_(std::move(model)),
                prompt_template_(std::move(promptTemplate)),
                chat_memory_(std::move(chatMemory)) {}

        StepFunctionPtr GetStepFunction() override {
            if (std::holds_alternative<LLMPtr>(model_)) {
                return this->output_converter_->AsInstructorFunction()
                       | chat_memory_->AsLoadMemoryFunction()
                       | prompt_template_
                       | std::get<LLMPtr>(model_)
                       | chat_memory_->AsSaveMemoryFunction();
            }
            if (std::holds_alternative<ChatModelPtr>(model_)) {
                return this->output_converter_->AsInstructorFunction()
                       | chat_memory_->AsLoadMemoryFunction()
                       | prompt_template_
                       | std::get<ChatModelPtr>(model_)
                       | chat_memory_->AsSaveMemoryFunction();
            }
            throw InstinctException("invalid model given to LLMChain");
        }


    };

    static MultilineChainPtr CreateMultilineChain(
            const LanguageModelVariant &model,
            const PromptTemplatePtr& prompt_template = nullptr,
            const ChainOptions& options = {},
            const ChatMemoryPtr& chat_memory = nullptr,
            InputParserPtr<PromptValue> input_parser = nullptr,
            OutputParserPtr<MultilineGeneration> output_parser = nullptr
            ) {
        if (!input_parser) {
            input_parser = std::make_shared<PromptValueInputParser>();
        }
        if (!output_parser) {
            output_parser = std::make_shared<MultilineGenerationOutputParse>();
        }
        return std::make_shared<TextChain>(
                input_parser,
                output_parser,
                model,
                prompt_template,
                chat_memory,
                options
        );
    }


    static TextChainPtr CreateTextChain(
            const LanguageModelVariant &model,
            const PromptTemplatePtr& prompt_template = nullptr,
            const ChainOptions& options = {},
            const ChatMemoryPtr& chat_memory = nullptr,
            InputParserPtr<PromptValue> input_parser = nullptr,
            OutputParserPtr<Generation> output_parser = nullptr
    ) {
        if (!input_parser) {
            input_parser = std::make_shared<PromptValueInputParser>();
        }
        if (!output_parser) {
            output_parser = std::make_shared<GenerationOutputParser>();
        }
        return std::make_shared<TextChain>(
                input_parser,
                output_parser,
                model,
                prompt_template,
                chat_memory,
                options
        );
    }


}


#endif //LLMCHAIN_HPP
