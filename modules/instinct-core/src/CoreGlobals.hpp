//
// Created by RobinQu on 2024/2/12.
//

#ifndef COREGLOBALS_H
#define COREGLOBALS_H

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#define INSTINCT_CORE_NS instinct::core

namespace INSTINCT_CORE_NS {
    template<class>
    inline constexpr bool always_false_v = false;

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template <typename R, typename V>
        concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;

}


#endif //COREGLOBALS_H
