//
// Created by RobinQu on 2024/2/17.
//


#include <gtest/gtest.h>
#include <concepts>

// template<typename Input, typename Output>
class RunFoo {
    auto Invoke(auto && input) {
        return 1;
    }

    auto operator()(auto && input) {
        return Invoke(input);
    }
};

auto call(std::invocable auto f) {
    return std::invoke(f);
}

auto chain_apply(auto&& a, auto&& f, auto&& ...fs) {
    if constexpr(sizeof...(fs)) {
        return chain_apply(std::forward_as_tuple(std::apply(f, a)), fs...);
    } else {
        return std::apply(f, a);
    }
}


TEST(TestFunction, TestChaining) {

    auto echo = [](const auto& str) {
        std::cout << str << std::endl;
    };

    auto double_integer = [](const auto& int_value) {
        return int_value * 2;
    };

    auto str_size = [](const auto& str) {
        return str.size();
    };

    // call([]() {
    //     std::cout << "hello" << std::endl;
    // });

    // chain_apply(std::tuple<std::string>("a"), double_integer, str_size, echo);

    // chain_apply(std::tuple<int>(0), RunFoo {}, RunFoo {});


}