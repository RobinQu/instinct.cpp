//
// Created by RobinQu on 2024/2/12.
//

#ifndef COREGLOBALS_H
#define COREGLOBALS_H

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <unicode/unistr.h>
#include <unicode/uversion.h>
#include <google/protobuf/message.h>

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

    using U32String = U_ICU_NAMESPACE::UnicodeString;

    template <typename T>
    concept is_pb_message = std::derived_from<T, google::protobuf::Message>;

}


#endif //COREGLOBALS_H
