//
// Created by RobinQu on 2024/3/3.
//

#ifndef ASSERTIONS_HPP
#define ASSERTIONS_HPP

#include "CoreGlobals.hpp"

// #include "StringUtils.hpp"


namespace INSTINCT_CORE_NS {
    template<typename R>
    concept sized_range = std::ranges::input_range<R> && std::ranges::sized_range<R>;

    template<typename T>
    concept numberic = std::integral<T> || std::floating_point<T>;

    template <typename R1, typename R2>
    requires sized_range<R1> && sized_range<R2>
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

    template <typename R>
    requires sized_range<R>
    static bool check_non_empty_range(R&& r) {
        return std::ranges::size(r) > 0;
    }

    template <typename R>
    requires sized_range<R>
    static void assert_non_empty_range(R&& r, const std::string& message = "range object should not be empty") {
        if (!check_non_empty_range(std::forward<R>(r))) {
            throw InstinctException(message);
        }
    }

    template <typename R1, typename R2>
    requires sized_range<R1> && sized_range<R2>
    static bool check_equal_size(R1&& r1, R2&& r2) {
        return std::ranges::size(std::forward<R1>(r1)) == std::ranges::size(std::forward<R2>(r2));
    }

    template <typename R1, typename R2>
    requires sized_range<R1> && sized_range<R2>
    static void assert_equal_size(R1&& r1, R2&& r2, const std::string& message = "two range objects should have equal size") {
        if (!check_equal_size(std::forward<R1>(r1), std::forward<R2>(r2))) {
            throw InstinctException(message);
        }
    }

    static bool check_gt(const numberic auto & a, const numberic auto & b) {
        return a>b;
    }

    static bool check_lt(const numberic auto & a, const numberic auto & b) {
        return a<b;
    }

    static bool check_gte(const numberic auto & a, const numberic auto & b) {
        return a >= b;
    }

    static bool check_lte(const numberic auto & a, const numberic auto &b) {
        return a<=b;
    }

    static bool check_positive(const numberic auto & a) {
        return a>0;
    }

    static void assert_positive(const numberic auto & a, const std::string& message = "value should be positive ") {
        if(!check_positive(a)) {
            throw InstinctException(message);
        }
    }

    static void assert_gt(const numberic auto & a, const numberic auto & b, const std::string& message = "value should be greater") {
        if(!check_gt(a,b)) {
            throw InstinctException(message);
        }
    }

    static void assert_lt(const numberic auto & a, const numberic auto & b, const std::string& message = "value should be less") {
        if(!check_lt(a,b)) {
            throw InstinctException(message);
        }
    }

    static void assert_lte(const numberic auto & a, const numberic auto & b, const std::string& message = "value should be less or equal") {
        if(!check_lte(a,b)) {
            throw InstinctException(message);
        }
    }

    static void assert_true(bool v, const std::string& message = "assertion failed") {
        if (!v) {
            throw InstinctException(message);
        }
    }

    static void assert_icu_status(UErrorCode status, const std::string& msg = "ICU operation failed") {
        if(U_FAILURE(status)) {
            throw InstinctException(msg);
        }
    }

    static void assert_icu_status(UErrorCode status, const icu_74::UnicodeString& msg = "ICU operation failed") {
        std::string msg_utf8;
        msg.toUTF8String(msg_utf8);
        assert_icu_status(status, msg_utf8);
    }



}

#endif //ASSERTIONS_HPP
