//
// Created by RobinQu on 2024/3/11.
//

#ifndef BASEOUTPUTPARSER_HPP
#define BASEOUTPUTPARSER_HPP

#include "IOutputParser.hpp"
#include "LLMGlobals.hpp"
#include "functional/StepFunctions.hpp"

namespace INSTINCT_LLM_NS {
    static std::string DEFAULT_FORMAT_INSTRUCTION_KEY = "format_instruction";

    template<typename T>
    class BaseOutputParser : public virtual IRunnable<JSONContextPtr, T>, public virtual IOutputParser<T> {
        std::string instruction_key_;

        /**
         * instruction function doesn't depend on any keys to output format instruction. Parsing relies on Generation from LLM output and doesn't read from context.
         * @return
         */
        StepFunctionPtr instruction_function_;
    public:
        explicit BaseOutputParser(std::string instruction_key = DEFAULT_FORMAT_INSTRUCTION_KEY)
                : instruction_key_(std::move(instruction_key)) {
            instruction_function_ = std::make_shared<LambdaStepFunction>(
                    [&](const JSONContextPtr &context) {
                        context->PutPrimitive(instruction_key_, GetFormatInstruction());
                        return context;
                    },
                    {},
                    {instruction_key_});
        }

        std::string GetFormatInstruction() override = 0;

        T ParseResult(const Generation &result) override = 0;

        T Invoke(const JSONContextPtr &input) override {
            return ParseResult(input);
        }

        StepFunctionPtr AsInstructorFunction() {
            return instruction_function_;
        }
    };

    template<typename T>
    using OutputParserPtr = std::shared_ptr<BaseOutputParser<T>>;

    using TextOutputParserPtr = OutputParserPtr<std::string>;
    using MultilineTextOutputParserPtr = OutputParserPtr<MultiLineText>;

}


#endif //BASEOUTPUTPARSER_HPP
