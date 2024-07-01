//
// Created by RobinQu on 2024/2/21.
//

#include <instinct/core_global.hpp>
#include <rpp/rpp.hpp>
#include <gtest/gtest.h>


namespace INSTINCT_CORE_NS::experimental::reactive_chain {

    template<typename Input, typename Output, typename InputRange=std::vector<Input>, typename OutputChunk=Output>
        requires RangeOf<InputRange, Input>
    class Chainable {
    public:
        virtual ~Chainable() = default;
        virtual Output Invoke(Input&& input) = 0;
        virtual rpp::dynamic_observable<Output> Batch(InputRange&& input_range)=0 ;
        virtual rpp::dynamic_observable<OutputChunk> Stream(Input&& input) = 0;
    };

    template<typename Input, typename Output>
    class BaseChainable : public Chainable<Input, Output> {
    public:
        rpp::dynamic_observable<Output> Batch(std::vector<Input>&& input_range) override {
            return rpp::source::create<Output>([&](const auto& subscriber) {
                            for(auto&& input: input_range) {
                                subscriber.on_next(this->Invoke(std::forward<Input>(input)));
                            }
                            subscriber.on_completed();
                        });
        }

        rpp::dynamic_observable<Output> Stream(Input&& input) override {
            return rpp::source::create<Output> ([&](const auto& subscriber) {
                subscriber.on_next(this->Invoke(std::forward<Input>(input)));
                subscriber.on_completed();
            });
        }
    };

    class StringChainable: public BaseChainable<std::string, std::string> {
    public:
        std::string Invoke(std::string&& prefix) override {
            return prefix + "hello_world";
        }
    };

    //
    // class test1 {
    // public:
    //     auto operator()(const std::string& str) {
    //         return str.size();
    //     }
    // };
    //
    // class test2 {
    //     auto invoke(const std::string& a) {
    //         return [&](const auto& observable) {
    //             return a + a;
    //         };
    //     }
    // };
    //

    TEST(RPPChain, Chaining) {
        auto c1 = StringChainable();
        auto ob = c1.Invoke("abc");
        c1.Batch({"No1: ", "No2: "})
        .subscribe([](const auto& str) {
           std::cout << str << std::endl;
        });
    }

    TEST(RPP, flat_map) {
        rpp::source::just(1,3,4,5)
            | rpp::operators::flat_map([](auto v) { return rpp::source::just(v * 2); })
            | rpp::operators::subscribe([](int i) {
                std::cout << i << std::endl;
            });
    }

}