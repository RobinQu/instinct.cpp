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

    template <typename R, typename V>
    concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;


    template <typename Input, typename Output, typename InputRange=std::vector<Input>, typename OutputRange=std::vector<Output>>
        requires RangeOf<InputRange, Input> && RangeOf<OutputRange, Output>
    class Chainable {
    public:
        virtual ~Chainable() = default;

        virtual Output Invoke(const Input& input) = 0;

        virtual OutputRange Batch(const InputRange& input) {
            const auto output_view =  input | std::views::transform([&](const auto& v) {
                return Invoke(v);
            });
            return {output_view.begin(), output_view.end()};
        }

        virtual OutputRange Stream(const Input& input) {
            return {std::forward<Output>(Invoke(input))};
        }
    };
    template <typename Input, typename Output>
    using ChainablePtr = std::shared_ptr<Chainable<Input, Output>>;

    template <typename Input, typename Intermediate, typename Output>
    class ChainablePair final: public Chainable<Input,Output> {
        ChainablePtr<Input, Intermediate> c1_;
        ChainablePtr<Intermediate, Output> c2_;

    public:
        ChainablePair(
            ChainablePtr<Input, Intermediate>& c1,
            ChainablePtr<Intermediate, Output>& c2
        ):
            c1_(std::forward<ChainablePtr<Input, Intermediate>>(c1)),
            c2_(std::forward<ChainablePtr<Intermediate, Output>>(c2))
            // c1_(c1), c2_(c2)
        {
        }

        Output Invoke(const Input& input) override {
            return c2_->Invoke(std::forward<Intermediate>(c1_->Invoke(input)));
        }
    };

    template <typename Input, typename Intermediate, typename Output>
    static ChainablePtr<Input, Output> operator|(
        ChainablePtr<Input, Intermediate> &lhs,
        ChainablePtr<Intermediate, Output> &rhs
    ) {
        return std::make_shared<ChainablePair<Input, Intermediate, Output>>(lhs, rhs);
    }


}



#endif //CHAINABLE_HPP
