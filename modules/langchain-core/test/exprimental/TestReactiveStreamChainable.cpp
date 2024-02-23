//
// Created by RobinQu on 2024/2/21.
//

#include "CoreGlobals.hpp"
#include <rpp/rpp.hpp>
#include <gtest/gtest.h>


namespace LC_CORE_NS::experimental::reactive_chain {

    template <typename R, typename V>
concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;


    template<typename Input, typename Output, typename ObserverStrategy=rpp::details::from_iterable_strategy<std::vector<Output>, rpp::schedulers::defaults::iteration_scheduler>, typename InputRange=std::vector<Input>, typename OutputChunk=Output>
        requires RangeOf<InputRange, Input> && rpp::constraint::observer_strategy<ObserverStrategy, Input>
    class Chainable {
    public:
        virtual ~Chainable() = default;
        virtual Output Invoke(Input&& input) = 0;
        virtual rpp::observable<Output, ObserverStrategy> Batch(InputRange&& input_range)=0 ;
        virtual rpp::observable<OutputChunk, ObserverStrategy> Stream(Input&& input) = 0;
    };

    template<typename Input, typename Output>
    class BaseChainable : public Chainable<Input, Output> {
    public:
        rpp::observable<Output, rpp::details::from_iterable_strategy<std::vector<Output>, rpp::schedulers::
        current_thread>> Batch(std::vector<Input>&& input_range) {
            return rpp::source::create<Output>([&](const auto& subscriber) {
                            for(const auto& input: input_range) {
                                subscriber.on_next(this->Invoke(std::forward<Input>(input)));
                            }
                            subscriber.on_completed();
                        });
        }

        rpp::observable<Output, rpp::details::from_iterable_strategy<std::vector<Output>, rpp::schedulers::
        current_thread>> Stream(Input&& input) {
            return rpp::source::create<Output> ([&](const auto& subscriber) {
                subscriber.on_next(this->Invoke(std::forward<Input>(input)));
                subscriber.on_completed();
            });
        }

    };


    class StringChainable: public BaseChainable<std::string, std::string> {
    public:

        std::string Invoke(std::string&& input) override {
            return "hello_world";
        }
    };


    class test1 {
    public:
        auto operator()(const std::string& str) {
            return str.size();
        }
    };

    class test2 {
        auto invoke(const std::string& a) {
            return [&](const auto& observable) {
                return a + a;
            };
        }
    };


    TEST(RPPChain, Chaining) {
        auto ob = StringChainable().Invoke("abc");


        // ob.subscribe([](const auto& v) {
        //     std::cout << v << std::endl;
        // });

        // auto prompt = Prompt();
        // auto model = Ollama();
        // auto parser = OutputParser();

        // Runnable { invoke(); batch(); stream(); }
        // Partial: Runnable<Input,Output> { upstream;  }

        // (prompt | parser | model) -> Runnable


        // singular element stream
        // rpp::source::just("hello_world")
        //     | rpp::operators::map(test1());

        // plural elements stream
        // rpp::source::from_iterable(std::vector{"hello", "world"});
        //  | prompt.batch()
        //  | llm.batch()





    }

}