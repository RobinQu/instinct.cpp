//
// Created by RobinQu on 2024/2/21.
//
#include <gtest/gtest.h>

#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS::experimental::fn_chain {

    template<typename Fn>
    class Chainable {
        Fn fn_;
        std::string name_;

    public:
        explicit Chainable(Fn&& fn, std::string name="")
            : fn_(std::forward<Fn>(fn)),
              name_(std::move(name)) {
        }

        template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
Output Invoke(const Input& input) {
            return fn_(input);
        }

        template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
        Output operator()(const Input& input) {
            return Invoke(input);
        }

        [[nodiscard]] const std::string& GetName() const {
            return name_;
        }

    };

    template<typename Fn>
    Chainable<Fn> create_chainable(Fn&& fn, const std::string& name="") {
        return Chainable<Fn>(fn, name);
    }

    template<typename Fn1, typename Fn2>
    auto operator|(Fn1&& fn1, Fn2 && fn2) {
        return Chainable([&](const auto& input) {
            return fn2(fn1(input));
        });
    }


    TEST(TestFnChainable, ChainingWithFunction) {
        auto fn_1 = [](const std::string& str) {return str.size();};
        auto fn_2 = [](const size_t val) {return val*2;};
        auto fn_3 = [](const std::string::size_type& l) {return std::vector{l};};
        auto fn_4 = [](const std::string& str) {
            auto v = str | std::views::transform([](unsigned char c) {
                return std::toupper(c);
            });
            return std::string(v.begin(),v.end());
        };
        auto fn_5 = [](const std::string& str) {
            return str + str;
        };

        auto xn_1 = create_chainable(fn_1) | fn_2;

        std::cout << xn_1.Invoke("hello") << std::endl;

        auto xn_2 = create_chainable(fn_5) | fn_1 | fn_2 | [](const std::string::size_type& n) {
            return std::vector {n};
        };

        for(const auto& i: xn_2.Invoke("hello")) {
            std::cout << i << std::endl;
        }

    }


}