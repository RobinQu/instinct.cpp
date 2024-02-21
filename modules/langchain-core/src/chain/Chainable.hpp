//
// Created by RobinQu on 2024/2/20.
//

#ifndef CHAINABLE_HPP
#define CHAINABLE_HPP

#include "CoreGlobals.h"

namespace LC_CORE_NS::chain {
//
//
//     template<typename Fn>
//     class Chainable {
//         Fn fn_;
//         std::string name_;
//
//     public:
//         explicit Chainable(Fn fn, std::string name="")
//             : fn_(std::move(fn)),
//               name_(std::move(name)) {
//         }
//
//         template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
// Output Invoke(const Input& input) {
//             return std::invoke<Fn>(fn_, input);
//         }
//
//         template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
//         Output operator()(const Input& input) {
//             return Invoke(input);
//         }
//
//         [[nodiscard]] const std::string& GetName() const {
//             return name_;
//         }
//
//     };
//
//     template<typename Fn>
//     Chainable<Fn> create_chainable(Fn&& fn, const std::string& name="") {
//         return Chainable<Fn>(fn, name);
//     }
//
//
//
    // template<typename T>
    // class Chainable {
    //     T value_;
    //
    // public:
    //     explicit Chainable(T value)
    //         : value_(std::move(value)) {
    //     }
    //
    //     template<typename Fn, typename Output=std::invoke_result_t<Fn, T>>
    //     Chainable<Output> transform(Fn&& fn) {
    //         return Chainable<Output>(std::forward<Output>(std::invoke(fn, value_)));
    //     }
    //
    // };




}



#endif //CHAINABLE_HPP
