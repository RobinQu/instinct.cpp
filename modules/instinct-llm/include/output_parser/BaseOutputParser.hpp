//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASEOUTPUTPARSER_HPP
#define BASEOUTPUTPARSER_HPP

#include <utility>

#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "functional/StepFunctions.hpp"
#include "functional/JSONContextPolicy.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    // static std::string DEFAULT_FORMAT_INSTRUCTION_KEY = "format_instruction";

    struct OutputParserOptions {
        // std::string format_instruction_output_key = DEFAULT_FORMAT_INSTRUCTION_KEY;
        // std::string generation_input_key = DEFAULT_ANSWER_OUTPUT_KEY;
    };

    template<typename T>
    class BaseOutputParser:
            public BaseRunnable<JSONContextPtr, T>,
            public IConfigurable<OutputParserOptions>,
                    public virtual IOutputParser<T> {
        OutputParserOptions options_;

        StepFunctionPtr instruction_function_;
//        StepFunctionPtr transform_function_;

    public:

        explicit BaseOutputParser(const OutputParserOptions& options)
                : options_(options) {

            // instruction function doesn't depend on any keys to output format instruction.
            instruction_function_ = std::make_shared<LambdaStepFunction>(
                    [&](const JSONContextPtr &context) {
                        context->ProducePrimitive(
                                this->GetFormatInstruction()
                        );
                        return context;
                    });
//            transform_function_ = std::make_shared<LambdaStepFunction>([&](const JSONContextPtr &context) {
//                auto generation = context->RequireMessage<Generation>();
//                this->ParseResult(generation);
//            });
        }


        void Configure(const OutputParserOptions &options) override {
            options_ = options;
        }

        [[nodiscard]] const OutputParserOptions& GetOptions() const {
            return options_;
        }

        T Invoke(const JSONContextPtr &input) override {
            auto generation = input->RequireMessage<Generation>();
            return this->ParseResult(generation);
        }

        [[nodiscard]] StepFunctionPtr AsInstructorFunction() const {
            return instruction_function_;
        }

        std::string GetFormatInstruction() override {
            return "";
        }
    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<BaseOutputParser<T>>;

}


#endif //BASEOUTPUTPARSER_HPP
