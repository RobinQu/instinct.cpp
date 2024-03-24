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
#include "output_parser/GenerationOutputParser.hpp"
#include "output_parser/MultilineGenerationOutputParser.hpp"
#include "input_parser/PromptValueVariantInputParser.hpp"
#include "output_parser/StringOutputParser.hpp"
#include "prompt/PlainPromptTemplate.hpp"


namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    // to adapt both LLM and ChatModel
    using LanguageModelVariant = std::variant<LLMPtr, ChatModelPtr>;

    /**
     * Create a
     * @tparam Input Input type
     * @tparam Output Output type
     * @param input_parser Paser for given input to context object
     * @param prompt_template PromptTemplate to generate prompts
     * @param model LLM or ChatModel
     * @param output_parser Output parser for model output
     * @param chat_memory IChatMemory to save and load memory
     * @param options Chain options
     * @return
     */
    template<typename Input,typename Output>
    static StepFunctionPtr CreateLLMChain(
            const InputParserPtr<Input>& input_parser,
            const LanguageModelVariant& model,
            const OutputParserPtr<Output>& output_parser,
            PromptTemplatePtr prompt_template = nullptr,
            const ChatMemoryPtr& chat_memory = nullptr,
            const ChainOptions& options = {}
            ) {
        if(!prompt_template) {
            prompt_template = CreatePlainPromptTemplate("{ " +  DEFAULT_QUESTION_INPUT_OUTPUT_KEY + " }");
        }

        StepFunctionPtr model_function;
        if (std::holds_alternative<LLMPtr>(model)) {
            model_function = std::get<LLMPtr>(model)->AsModelFunction();
        }
        if (std::holds_alternative<ChatModelPtr>(model)) {
            model_function = std::get<ChatModelPtr>(model)->AsModelfunction();
        }
        assert_true(model_function, "should contain model function");

        StepFunctionPtr step_function;
        if (chat_memory) {
            auto context_fn = CreateMappingStepFunction({
                {"format_instruction", FunctionReducer(output_parser->AsInstructorFunction())},
                {"chat_history", FunctionReducer(chat_memory->AsLoadMemoryFunction())},
                {"question", GetterReducer("question")}
            });


            return CreateMappingStepFunction({
              {"answer", FunctionReducer(context_fn | prompt_template | model_function)},
              {"question", GetterReducer("question")}
            })
            | chat_memory->AsSaveMemoryFunction({.prompt_variable_key="question", .answer_variable_key="answer"})
             // | CreateGetterReducer("answer");

        } else {
            step_function = output_parser->AsInstructorFunction()
                            | prompt_template
                            | model_function;
        }

        return std::make_shared<FunctionalMessageChain<Input,Output>>(
                input_parser,
                output_parser,
                step_function,
                options
        );
    }


    static MultilineChainPtr CreateMultilineChain(
            const LanguageModelVariant &model,
            const PromptTemplatePtr& prompt_template = nullptr,
            const ChainOptions& options = {},
            const ChatMemoryPtr& chat_memory = nullptr,
            InputParserPtr<PromptValueVariant> input_parser = nullptr,
            OutputParserPtr<MultilineGeneration> output_parser = nullptr
            ) {
        if(!input_parser) {
            input_parser = std::make_shared<PromptValueVariantInputParser>();
        }
        if (!output_parser) {
            output_parser = std::make_shared<MultilineGenerationOutputParse>();
        }
        return CreateLLMChain<PromptValueVariant, MultilineGeneration> (
                input_parser,
                model,
                output_parser,
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
            InputParserPtr<PromptValueVariant> input_parser = nullptr,
            OutputParserPtr<std::string> output_parser = nullptr
    ) {
        if(!input_parser) {
            input_parser = std::make_shared<PromptValueVariantInputParser>();
        }
        if (!output_parser) {
            output_parser = std::make_shared<StringOutputParser>();
        }


        return CreateLLMChain<PromptValueVariant, std::string>(
            input_parser,
            model,
            output_parser,
            prompt_template,
            chat_memory,
            options
        );
    }


}


#endif //LLMCHAIN_HPP
