//
// Created by RobinQu on 2024/2/21.
//
#include <gtest/gtest.h>

#include <instinct/CoreGlobals.hpp>


namespace INSTINCT_CORE_NS::exprimental::io_chain {
    template<typename R, typename V>
    concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;


    template<typename Input, typename Output, typename InputRange=std::vector<Input>, typename OutputRange=std::vector<
        Output>>
        requires RangeOf<InputRange, Input> && RangeOf<OutputRange, Output>
    class Chainable {
    public:
        virtual ~Chainable() = default;

        virtual Output Invoke(const Input& input) = 0;

        virtual Output operator()(const Input& input) {
            return Invoke(input);
        }

        virtual OutputRange Batch(const InputRange& input) {
            const auto output_view = input | std::views::transform([&](const auto& v) {
                return Invoke(v);
            });
            return {output_view.begin(), output_view.end()};
        }

        virtual OutputRange Stream(const Input& input) {
            return {std::forward<Output>(Invoke(input))};
        }
    };

    template<typename Input, typename Output>
    using ChainablePtr = std::shared_ptr<Chainable<Input, Output>>;

    template<typename Input, typename Intermediate, typename Output>
    class ChainablePair final : public Chainable<Input, Output> {
        ChainablePtr<Input, Intermediate> c1_;
        ChainablePtr<Intermediate, Output> c2_;

    public:
        ChainablePair(
            ChainablePtr<Input, Intermediate>& c1,
            ChainablePtr<Intermediate, Output>& c2
        ): c1_(std::forward<ChainablePtr<Input, Intermediate>>(c1)),
           c2_(std::forward<ChainablePtr<Intermediate, Output>>(c2))
        // c1_(c1), c2_(c2)
        {
        }

        Output Invoke(const Input& input) override {
            return c2_->Invoke(std::forward<Intermediate>(c1_->Invoke(input)));
        }
    };



    template<typename Fn1, typename Fn2>
    class PartialChain {
        Fn1 fn1_;
        Fn2 fn2_;

    public:
        PartialChain(Fn1&& fn1, Fn2&& fn2)
            : fn1_(std::forward<Fn1>(fn1)),
              fn2_(std::forward<Fn2>(fn2)) {
        }

        template<typename Input, typename Intermediate=std::invoke_result_t<Fn1, Input>, typename Output=
            std::invoke_result_t<Fn2, Intermediate>>
        Output Invoke(Input&& input) {
            return fn2_(fn1_(std::forward<Input>(input)));
            // return std::invoke<Fn2>(fn2_, std::forward<Intermediate>(
            //     std::invoke<Fn1>(fn1_, std::forward<Input>(input))
            // ));
        }

        template<typename Input, typename Intermediate=std::invoke_result_t<Fn1, Input>, typename Output=
            std::invoke_result_t<Fn2, Intermediate>>
        Output operator()(Input&& input) {
            return Invoke(input);
        }
    };


    class DoubleCopyString final : public Chainable<std::string, std::string> {
    public:
        std::string Invoke(const std::string& input) override {
            return input + input;
        }

        static ChainablePtr<std::string, std::string> Create() {
            return std::make_shared<DoubleCopyString>();
        }
    };


    class CalculateStringLength final : public Chainable<std::string, std::string::size_type> {
    public:
        using Ptr = ChainablePtr<std::string, std::string::size_type>;

        unsigned long Invoke(const std::string& input) override {
            return input.size();
        }

        static Ptr Create() {
            return std::make_shared<CalculateStringLength>();
        }
    };

    namespace function_chain {

//        template<typename Fn1, typename Fn2, typename Input, typename Intermediate = std::invoke_result_t<Fn1, Input>, typename Output = std::invoke_result_t<Fn2, Intermediate>>
//        static PartialChain<Fn1, Fn2> operator|(Fn1&& fn1, Fn2&& fn2) {
//            return PartialChain<Fn1, Fn2>(std::forward<Fn1>(fn1), std::forward<Fn2>(fn2));
//        }
//
//        TEST(TestInputOutputChain, FunctionChaining) {
//            auto fn_1 = [](const std::string& str) { return str.size(); };
//            auto fn_2 = [](const size_t val) { return val * 2; };
//            auto fn_3 = [](const std::string::size_type& l) { return std::vector{l}; };
//            auto fn_4 = [](const std::string& str) {
//                auto v = str | std::views::transform([](unsigned char c) {
//                    return std::toupper(c);
//                });
//                return std::string(v.begin(), v.end());
//            };
//            auto fn_5 = [](const std::string& str) {
//                return str + str;
//            };
//
//            auto v1 = DoubleCopyString();
//            auto v2 = CalculateStringLength();
//            auto chain2 = v1 | v2;
//            chain2.Invoke("hello_world");
//
//
//            auto chain3 = v1 | v2 | fn_3;
//            // std::cout <<  << std::endl;
//            for (const auto& c: chain3.Invoke("hello_world")) {
//                std::cout << c << std::endl;
//            }
//        }
    }



    namespace ptr_chain {
        template<typename Input, typename Intermediate, typename Output>
        static ChainablePtr<Input, Output> operator|(
                ChainablePtr<Input, Intermediate>& lhs,
                ChainablePtr<Intermediate, Output>& rhs
        ) {
            return std::make_shared<ChainablePair<Input, Intermediate, Output>>(lhs, rhs);
        }
        TEST(TestInputOutputChain, PtrChaining) {
            auto n1 = DoubleCopyString::Create();
            auto n2 = CalculateStringLength::Create();
            auto chain1 = n1 | n2;
            std::cout << chain1->Invoke("hello_world") << std::endl;
        }
    }




}


