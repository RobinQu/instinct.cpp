//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MESSAGECHAIN_HPP
#define INSTINCT_MESSAGECHAIN_HPP

#include "functional/RunnableChain.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "output_parser/BaseOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    struct ChainOptions {
        std::string input_prompt_variable_key = DEFAULT_PROMPT_INPUT_KEY;
        std::string output_answer_variable_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    template<
            typename Input,
            typename Output,
            typename Context = JSONContextPtr,
            typename InputConverter = InputParserPtr<Input>,
            typename OutputConverter = OutputParserPtr<Output>
    >
    requires is_pb_message<Input> && is_pb_message<Output>
    class MessageChain: public BaseRunnable<Input, Output> {
    protected:
        InputConverter input_converter_;
        OutputConverter output_converter_;
        ChainOptions options_;
    public:
        MessageChain(const InputParserPtr<Input> &inputConverter,
                     const OutputParserPtr<Output> &outputConverter,
                     const ChainOptions& options = {}
                     ) :
                    BaseRunnable<Input, Output>(),
                     input_converter_(inputConverter),
                     output_converter_(outputConverter),
                    options_(options) {}

        const ChainOptions& GetOptions() const {
            return options_;
        }

        virtual StepFunctionPtr GetStepFunction() = 0;

        InputParserPtr<Input> GetInputParser() {
            return input_converter_;
        }

        OutputParserPtr<Output> GetOutputParser() {
            return output_converter_;
        }

        Output Invoke(const Input &input) override {
            auto ctx = input_converter_->Invoke(input);
            auto step = GetStepFunction();
            auto output = step->Invoke(ctx);
            return output_converter_->Invoke(output);
        }
    };


    template<
            typename Input,
            typename Output
    >
    using MessageChainPtr = std::shared_ptr<RunnableChain<Input, Output>>;
}


#endif //INSTINCT_MESSAGECHAIN_HPP
