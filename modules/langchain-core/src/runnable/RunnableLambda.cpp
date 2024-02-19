//
// Created by RobinQu on 2024/2/16.
//

#include "RunnableLambda.h"

namespace LC_CORE_NS {
    template<typename Input, typename Output>
    Output RunnableLambda<Input, Output>::Invoke(const Input& input) {
        return std::invoke(fn_, std::forward<Input>(input));
    }

    template<typename Input, typename Output>
    std::initializer_list<Output> RunnableLambda<Input, Output>::
    Batch(const std::initializer_list<Input>& input_range) {

    }

    template<typename Input, typename Output>
    std::initializer_list<Output> RunnableLambda<Input, Output>::Stream(const Input& input) {
    }
} // LC_CORE_NS