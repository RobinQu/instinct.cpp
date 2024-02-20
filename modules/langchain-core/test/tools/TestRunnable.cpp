//
// Created by RobinQu on 2024/2/15.
//

#include <gtest/gtest.h>
//
#include "CoreTypes.h"
#include "chain/Chainable.hpp"

// auto map_callables(std::map<std::string, F>) {}
using namespace LC_CORE_NS;


class DoubleCopyString final: public chain::Chainable<std::string, std::string> {
public:
    std::string Invoke(const std::string& input) override {
        return input + input;
    }

    static chain::ChainablePtr<std::string, std::string> Create() {
        return std::make_shared<DoubleCopyString>();
    }
};


class CalculateStringLength final: public chain::Chainable<std::string, std::string::size_type> {
public:
    using Ptr = chain::ChainablePtr<std::string, std::string::size_type>;

    unsigned long Invoke(const std::string& input) override {
        return input.size();
    }

    static Ptr Create() {
        return std::make_shared<CalculateStringLength>();
    }
};


TEST(TestRunnable, TestPipeline) {

    auto fn_1 = [](const std::string& str) {return str.size();};
    auto fn_2 = [](const size_t val) {return val*2;};
    auto fn_3 = [](const long& l) {return std::vector{l};};
    auto fn_4 = [](const std::string& str) {
        auto v = str | std::views::transform([](unsigned char c) {
            return std::toupper(c);
        });
        return std::string(v.begin(),v.end());
    };
    auto fn_5 = [](const std::string& str) {
        return str + str;
    };
    // using ArgType = first_argument<decltype(fn_1)>;
    // auto n_1 = chain::create_chainable(fn_1);

    auto n1 = DoubleCopyString::Create();
    auto n2 = CalculateStringLength::Create();
    auto chain = n1 | n2;
    std::cout << chain->Invoke("hello_world") << std::endl;

    // n_1.Invoke("123");

    // std::function<std::any()> x_1 = []() {return 1;};
    // using Ret = std::invoke_result_t<decltype(x_1)>;
    // std::cout << typeid(Ret).name() << std::endl;
    // xn_1 : std::function<std::any()>

    // xn_1 : std::function<State(const State&)>
    // map_fn: std:function<std::any(const std::any&>);
    // BaseLLM { MessagePtr Invoke(PromptPtr); }
    // map_result r;
    // r.put('k', '111');
    // r.get<typename std::string>('k')

    // fn: std::function<Input> -> map{std::string, X}

    // auto xn_4 = create_node(fn_4, "fn_4");
    // auto xn_5 = create_node(fn_5, "fn_5");
    // using T = decltype(fn_4);
    // JoinFunctionsGraphNode<T> map_node;
    // map_node.AddPort("fn_4", fn_4);
    // map_node.AddPort("fn_5", fn_5);
    //




    // (fn_4 + fn_4) |

    // struct T {std::string a, std::vector<Doc> docs;}
    // create_map({"x_2": xn_1, "x_2": xn_2}) -> map_result
    // combie






}