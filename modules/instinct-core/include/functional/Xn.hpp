//
// Created by RobinQu on 2024/3/24.
//

#ifndef FN_HPP
#define FN_HPP

#include "CoreGlobals.hpp"
#include "StepFunctions.hpp"


namespace INSTINCT_CORE_NS {

    namespace xn {
        namespace steps {
            static StepFunctionPtr sequence(const std::vector<StepFunctionPtr> &steps) {
                return std::make_shared<SequenceStepFunction>(steps);
            }

            static StepFunctionPtr mapping(const std::unordered_map<std::string, JSONContextReducer>& steps) {
                return std::make_shared<MappingStepFunction>(steps);
            }

            static StepFunctionPtr reducer(const std::string& name) {
                return std::make_shared<ReducerStepFunction>(name);
            }

            static StepFunctionPtr empty() {
                return std::make_shared<EmptyStepFunction>();
            }

        }
        namespace reducers {
            static JSONContextReducer return_value(const StepFunctionPtr& step_function) {
                return FunctionReducer(step_function);
            }

            static JSONContextReducer selection(const std::string& name) {
                return GetterReducer(name);
            }
        }
    }

    inline StepFunctionPtr operator|(const StepFunctionPtr &first, const StepFunctionPtr &second) {
        std::vector<StepFunctionPtr> steps;
        if (auto first_as_steps = std::dynamic_pointer_cast<SequenceStepFunction>(first)) {
            // try to collapse first as SequenceStepFunction
            steps.insert(steps.end(), first_as_steps->GetSteps().begin(), first_as_steps->GetSteps().end());
        } else if (first != nullptr) {
            steps.push_back(first);
        }

        // try to collapse second as SequenceStepFunction
        if (auto second_as_steps = std::dynamic_pointer_cast<SequenceStepFunction>(second)) {
            steps.insert(steps.end(), second_as_steps->GetSteps().begin(), second_as_steps->GetSteps().end());
        } else if (second != nullptr) {
            steps.push_back(second);
        }
        if (steps.empty()) {
            return std::make_shared<EmptyStepFunction>();
        }
        return std::make_shared<SequenceStepFunction>(steps);
    }

    inline StepFunctionPtr operator|(const StepFunctionPtr &first, const StepLambda &step_lambda) {
        return first | std::make_shared<LambdaStepFunction>(step_lambda);
    }

    inline StepFunctionPtr operator|(
        std::initializer_list<std::pair<const std::string, JSONContextReducer>>&& first,
        const StepFunctionPtr &second
    ) {
        const auto mapping = std::make_shared<MappingStepFunction>(first);
        return mapping | second;
    }

    inline StepFunctionPtr operator|(
        const StepFunctionPtr &first,
        std::initializer_list<std::pair<const std::string, JSONContextReducer>>&& second
    ) {
        const auto mapping = std::make_shared<MappingStepFunction>(second);
        return first | mapping;
    }

}

#endif //FN_HPP
