//
// Created by RobinQu on 2024/2/23.
//

#ifndef FUNCTIONUTILS_HPP
#define FUNCTIONUTILS_HPP

#include <instinct/core_global.hpp>


namespace INSTINCT_CORE_NS {

    struct FunctionUtils final{

        template<typename Fn, typename Arg, typename Ret=std::invoke_result_t<Fn, Arg>, typename ArgRange=std::vector<Arg>, typename RetRange=std::vector<Ret>>
        requires RangeOf<ArgRange, Arg> && RangeOf<RetRange, Ret> && std::is_invocable_v<Fn, Arg>
        static RetRange PerformBatch(Fn&& fn, ArgRange&& args) {
            return args | std::views::transform([&](auto&& arg) {
                return std::invoke(std::forward<Fn>(fn), std::forward<Arg>(arg));
            });
        }

    };

}


#endif //FUNCTIONUTILS_HPP
