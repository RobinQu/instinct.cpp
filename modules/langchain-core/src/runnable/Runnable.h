//
// Created by RobinQu on 2024/2/14.
//

#ifndef RUNNABLE_H
#define RUNNABLE_H

#include "CoreGlobals.h"
#include <ranges>

namespace LC_CORE_NS {

template <typename R, typename V>
concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;



template<typename Input, typename Output, typename InputRange = std::initializer_list<Input>, typename OutputRange = std::initializer_list<Output>>
    requires RangeOf<OutputRange, Output> && RangeOf<InputRange, Input>
class Runnable {
public:
    using input_type = Input;
    using output_type = Output;

    Runnable(Runnable&&) = delete;
    Runnable(const Runnable&) = delete;

    virtual ~Runnable() = default;
    virtual Output Invoke(const Input& input) = 0;
    virtual OutputRange Batch(const InputRange& input_range) = 0;
    virtual OutputRange Stream(const Input& input) = 0;
};

} // LC_CORE_NS

#endif //RUNNABLE_H
