//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASEOUTPUTPARSER_HPP
#define BASEOUTPUTPARSER_HPP

#include <utility>

#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "functional/StepFunctions.hpp"

namespace INSTINCT_LLM_NS {
    static std::string DEFAULT_FORMAT_INSTRUCTION_KEY = "format_instruction";


    struct OutputParserOptions {
        std::string format_instruction_output_key = DEFAULT_FORMAT_INSTRUCTION_KEY;
        std::string generation_input_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    template<typename T>
    class BaseOutputParser: public BaseRunnable<JSONContextPtr, T>, public virtual IOutputParser<T> {
        OutputParserOptions options_;

        StepFunctionPtr instruction_function_;

    public:


        explicit BaseOutputParser(OutputParserOptions options)
                : options_(std::move(options)) {

            // instruction function doesn't depend on any keys to output format instruction.
            instruction_function_ = std::make_shared<LambdaStepFunction>(
                    [&](const JSONContextPtr &context) {
                        context->PutPrimitive(
                                options_.format_instruction_output_key,
                                this->GetFormatInstruction()
                                );
                        return context;
                    },
                    {},
                    {options_.format_instruction_output_key});
        }


        [[nodiscard]] const OutputParserOptions& GetOptions() const {
            return options_;
        }

//        std::string GetFormatInstruction() override = 0;

//        T ParseResult(const JSONContextPtr &result) override = 0;

        T Invoke(const JSONContextPtr &input) override {
            return this->ParseResult(input);
        }

        [[nodiscard]] StepFunctionPtr AsInstructorFunction() const {
            return instruction_function_;
        }
    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<BaseOutputParser<T>>;

}


#endif //BASEOUTPUTPARSER_HPP
