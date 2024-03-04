//
// Created by RobinQu on 2024/3/3.
//

#ifndef ASSERTIONS_HPP
#define ASSERTIONS_HPP

#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {



    template <typename R1, typename R2>
    requires std::ranges::input_range<R1> && std::ranges::sized_range<R1>
        && std::ranges::input_range<R2> && std::ranges::sized_range<R2>
    static bool check_equality(R1&& a, R2&& b) {
        if (std::ranges::size(a) != std::ranges::size(b)) {
            return false;
        }
        auto itr_a = a.begin();
        auto itr_b = b.begin();
        while(itr_a != a.end() && itr_b != b.end()) {
            if(*itr_a != *itr_b) {
                return false;
            }
            ++itr_a;
            ++itr_b;
        }
        return true;
    }


}

#endif //ASSERTIONS_HPP
