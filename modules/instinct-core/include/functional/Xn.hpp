//
// Created by RobinQu on 2024/3/24.
//

#ifndef FN_HPP
#define FN_HPP

#include "CoreGlobals.hpp"
#include "StepFunctions.hpp"


namespace xn {
    using namespace INSTINCT_CORE_NS;

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

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(const INSTINCT_CORE_NS::StepFunctionPtr &first, const INSTINCT_CORE_NS::StepFunctionPtr &second) {
    std::vector<INSTINCT_CORE_NS::StepFunctionPtr> steps;
    if (auto first_as_steps = std::dynamic_pointer_cast<INSTINCT_CORE_NS::SequenceStepFunction>(first)) {
        // try to collapse first as SequenceStepFunction
        steps.insert(steps.end(), first_as_steps->GetSteps().begin(), first_as_steps->GetSteps().end());
    } else if (first != nullptr) {
        steps.push_back(first);
    }

    // try to collapse second as SequenceStepFunction
    if (auto second_as_steps = std::dynamic_pointer_cast<INSTINCT_CORE_NS::SequenceStepFunction>(second)) {
        steps.insert(steps.end(), second_as_steps->GetSteps().begin(), second_as_steps->GetSteps().end());
    } else if (second != nullptr) {
        steps.push_back(second);
    }
    if (steps.empty()) {
        return std::make_shared<INSTINCT_CORE_NS::EmptyStepFunction>();
    }
    return std::make_shared<INSTINCT_CORE_NS::SequenceStepFunction>(steps);
}

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(const INSTINCT_CORE_NS::StepFunctionPtr &first, const INSTINCT_CORE_NS::StepLambda &step_lambda) {
    return first | std::make_shared<INSTINCT_CORE_NS::LambdaStepFunction>(step_lambda);
}

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(
        std::initializer_list<std::pair<const std::string, INSTINCT_CORE_NS::JSONContextReducer>>&& first,
        const INSTINCT_CORE_NS::StepFunctionPtr &second
) {
    const auto mapping = std::make_shared<INSTINCT_CORE_NS::MappingStepFunction>(first);
    return mapping | second;
}

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(
        const INSTINCT_CORE_NS::StepFunctionPtr &first,
        std::initializer_list<std::pair<const std::string, INSTINCT_CORE_NS::JSONContextReducer>>&& second
) {
    const auto mapping = std::make_shared<INSTINCT_CORE_NS::MappingStepFunction>(second);
    return first | mapping;
}




#endif //FN_HPP
