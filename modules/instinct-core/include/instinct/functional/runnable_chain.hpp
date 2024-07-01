//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_RUNNABLECHAIN_HPP
#define INSTINCT_RUNNABLECHAIN_HPP

#include <instinct/core_global.hpp>
#include <instinct/functional/step_functions.hpp>
#include <instinct/functional/json_context.hpp>
#include <instinct/functional/runnable.hpp>

namespace INSTINCT_CORE_NS {


    template<
            typename Input,
            typename Output,
            typename Context = JSONContextPtr,
            typename InputConverter = RunnablePtr<Input, Context>,
            typename OutputConverter = RunnablePtr<Context, Output>
    >
    class RunnableChain final: public BaseRunnable<Input, Output> {
        InputConverter input_converter_;
        OutputConverter output_converter_;
        StepFunctionPtr step_function_{};
    public:
        RunnableChain(InputConverter input_converter, OutputConverter output_converter, StepFunctionPtr step_function)
                : input_converter_(std::move(input_converter)),
                  output_converter_(std::move(output_converter)),
                  step_function_(std::move(step_function)) {
        }

        [[nodiscard]] StepFunctionPtr GetStepFunction() const {
            return step_function_;
        }

        Output Invoke(const Input& input) override {
            auto ctx = input_converter_->Invoke(input);
            step_function_->Invoke(ctx);
            return output_converter_->Invoke(ctx);
        }
    };


}

#endif //INSTINCT_RUNNABLECHAIN_HPP
