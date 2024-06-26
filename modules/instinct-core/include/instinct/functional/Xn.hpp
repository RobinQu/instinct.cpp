//
// Created by RobinQu on 2024/3/24.
//

#ifndef FN_HPP
#define FN_HPP

#include "CoreGlobals.hpp"
#include "StepFunctions.hpp"


namespace xn {
    using namespace INSTINCT_CORE_NS;
    using context_function_map = std::unordered_map<std::string, StepFunctionPtr>;
    namespace steps {
        static StepFunctionPtr sequence(const std::vector<StepFunctionPtr> &steps) {
            return std::make_shared<SequenceStepFunction>(steps);
        }

        static StepFunctionPtr mapping(const context_function_map& steps) {
            return std::make_shared<MappingStepFunction>(steps);
        }

        static StepFunctionPtr selection(const std::string& name) {
            return std::make_shared<SelectionStepFunction>(name);
        }

        static StepFunctionPtr passthrough() {
            return std::make_shared<PassthroughStepFunction>();
        }

        static StepFunctionPtr lambda(const StepLambda& lambda) {
            return std::make_shared<LambdaStepFunction>(lambda);
        }

        static StepFunctionPtr branch(const StepLambda& condition, const StepFunctionPtr& branch_a, const StepFunctionPtr& branch_b) {
            return std::make_shared<BranchStepFunction>(condition, branch_a, branch_b);
        }

    }
}

/**
 * pipe operator for two StepFunctionPtr
 * @param first
 * @param second
 * @return
 */
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
        return std::make_shared<INSTINCT_CORE_NS::PassthroughStepFunction>();
    }
    return std::make_shared<INSTINCT_CORE_NS::SequenceStepFunction>(steps);
}

/**
 * Pipe operator for StepLambda on right hand
 * @param first
 * @param step_lambda
 * @return
 */
inline INSTINCT_CORE_NS::StepFunctionPtr operator|(const INSTINCT_CORE_NS::StepFunctionPtr &first, const INSTINCT_CORE_NS::StepLambda &step_lambda) {
    return first | std::make_shared<INSTINCT_CORE_NS::LambdaStepFunction>(step_lambda);
}

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(
        const xn::context_function_map& first,
        const INSTINCT_CORE_NS::StepFunctionPtr &second
) {
    const auto mapping = std::make_shared<INSTINCT_CORE_NS::MappingStepFunction>(first);
    return mapping | second;
}

inline INSTINCT_CORE_NS::StepFunctionPtr operator|(
        const INSTINCT_CORE_NS::StepFunctionPtr &first,
        const xn::context_function_map& second
) {
    const auto mapping = std::make_shared<INSTINCT_CORE_NS::MappingStepFunction>(second);
    return first | mapping;
}




#endif //FN_HPP
