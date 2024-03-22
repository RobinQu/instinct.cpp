//
// Created by RobinQu on 3/22/24.
//

#ifndef INSTINCT_MESSAGECHAIN_HPP
#define INSTINCT_MESSAGECHAIN_HPP

#include <utility>

#include "functional/RunnableChain.hpp"
#include "input_parser/BaseInputParser.hpp"
#include "output_parser/BaseOutputParser.hpp"

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;

    struct ChainOptions {
    };

    template<
            typename Output,
            typename Input = JSONContextPtr,
            typename OutputConverter = OutputParserPtr<Output>
    >
    requires is_pb_message<Output>
    class MessageChain: public BaseRunnable<Input, Output> {
    protected:
        OutputConverter output_converter_;
        ChainOptions options_;
    public:
        explicit MessageChain(const OutputParserPtr<Output> &outputConverter,
                     const ChainOptions& options = {}) :
                    BaseRunnable<Input, Output>(),
                     output_converter_(outputConverter),
                    options_(options) {}

        [[nodiscard]] const ChainOptions& GetOptions() const {
            return options_;
        }

        virtual StepFunctionPtr GetStepFunction() = 0;

        OutputParserPtr<Output> GetOutputParser() {
            return output_converter_;
        }

        Output Invoke(const Input &input) override {
            auto step = GetStepFunction();
            auto output = step->Invoke(input);
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
