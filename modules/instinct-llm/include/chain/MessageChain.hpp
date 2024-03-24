//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MESSAGECHAIN_HPP
#define INSTINCT_MESSAGECHAIN_HPP

#include <utility>

#include "functional/RunnableChain.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "output_parser/BaseOutputParser.hpp"
#include "model/ILanguageModel.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    class GenerationToStringFunction final: public BaseStepFunction {
    public:
        std::vector<std::string> GetInputKeys() const override {
            return {};
        }

        std::vector<std::string> GetOutputKeys() const override {
            return {};
        }

    public:
        JSONContextPtr Invoke(const JSONContextPtr &input) override {
            auto generation = input->RequireMessage<Generation>();
            input->ProducePrimitive(generation.has_message() ? generation.message().content() : generation.text());
            return input;
        }
    };


    /**
     * MessageChain is kind of one-path runnable. Underlying `StepFunction` always expects a string prompt and returns a `Generation`, so that it works with `InputParser` and `OutputParser`.
     * @tparam Input
     * @tparam Output
     * @tparam InputParser
     * @tparam OutputParser
     */
    template<
            typename Input,
            typename Output,
            typename InputParser = InputParserPtr<Input>,
            typename OutputParser = OutputParserPtr<Output>
    >
    class MessageChain : public BaseRunnable<Input, Output> {
    protected:
        InputParser input_parser_;
        OutputParser output_parser_;
        ChainOptions options_;
    public:
        explicit MessageChain(
                const InputParserPtr<Input> &input_parser,
                const OutputParserPtr<Output> &output_parser,
                const ChainOptions &options = {}) :
                BaseRunnable<Input, Output>(),
                input_parser_(input_parser),
                output_parser_(output_parser),
                options_(options) {
            assert_true(input_parser_, "should have InputParser");
            assert_true(output_parser_, "should have OutputParser");
        }

        [[nodiscard]] const ChainOptions &GetOptions() const {
            return options_;
        }

        [[nodiscard]] virtual StepFunctionPtr GetStepFunction() const = 0;

        OutputParserPtr<Output> GetOutputParser() const {
            return output_parser_;
        }

        InputParserPtr<Input> GetInputParser() const {
            return input_parser_;
        }

        Output Invoke(const Input &input) override {
            auto context = input_parser_->Invoke(input);
            auto step = GetStepFunction();
            auto output = step->Invoke(context);
            return output_parser_->Invoke(output);
        }
    };

    template<
            typename Input,
            typename Output
    >
    class FunctionalMessageChain final : public MessageChain<Input, Output> {
        StepFunctionPtr step_function_;
    public:
        FunctionalMessageChain(const InputParserPtr<Input> &input_parser,
                           const OutputParserPtr<Output> &output_parser,
                           StepFunctionPtr  stepFunction,
                           const ChainOptions &options
                           ) :
                MessageChain<Input,Output>(input_parser,
                                           output_parser,
                                           options),
                step_function_(std::move(stepFunction)) {}

        [[nodiscard]] StepFunctionPtr GetStepFunction() const override {
            return step_function_;
        }
    };

    /**
     * universal pointer for all chains
     */
//    template<typename Input, typename Output>
//    using MessageChainPtr = std::shared_ptr<MessageChain<Input, Output>>;

    /**
     *  simple chain with variant input and string output
     */
    using TextChain = MessageChain<PromptValueVariant, std::string>;

//    class TextChain: public BaseRunnable<PromptValueVariant, std::string> {

//    };

    /**
     * pointer to TextChain
     */
    using TextChainPtr = std::shared_ptr<TextChain>;

    /**
     * Simple chain with variant input and multi-line text output
     */
    using MultilineChain = MessageChain<PromptValueVariant, MultilineGeneration>;

//    class MultilineChain: public BaseRunnable<PromptValueVariant, MultilineGeneration> {
//
//    };

    /**
     * Pointer to MultilineChain
     */
    using MultilineChainPtr = std::shared_ptr<MultilineChain>;


}


#endif //INSTINCT_MESSAGECHAIN_HPP
