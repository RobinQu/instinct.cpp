//
// Created by RobinQu on 2024/2/16.
//

#include "RunnableParallel.h"

namespace LC_CORE_NS {
    template<typename Input, typename Output>
    Output RunnableParallel<Input, Output>::Invoke(const Input& input) {
    }

    template<typename Input, typename Output>
    std::initializer_list<Output> RunnableParallel<Input, Output>::Batch(
        const std::initializer_list<Input>& input_range) {
    }

    template<typename Input, typename Output>
    std::initializer_list<Output> RunnableParallel<Input, Output>::Stream(const Input& input) {
    }
} // LC_CORE_NS
