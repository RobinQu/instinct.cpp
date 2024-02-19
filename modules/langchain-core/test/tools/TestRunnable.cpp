//
// Created by RobinQu on 2024/2/15.
//

#include <gtest/gtest.h>
//

#include <flowgraph/GraphNode.h>
#include "CoreTypes.h"
#include <cctype>

// auto map_callables(std::map<std::string, F>) {}
using namespace LC_CORE_NS;


template<typename Ret, typename Arg, typename... Rest>
Arg first_argument_helper(Ret(*) (Arg, Rest...));

template<typename Ret, typename F, typename Arg, typename... Rest>
Arg first_argument_helper(Ret(F::*) (Arg, Rest...));

template<typename Ret, typename F, typename Arg, typename... Rest>
Arg first_argument_helper(Ret(F::*) (Arg, Rest...) const);

template <typename F>
decltype(first_argument_helper(&F::operator())) first_argument_helper(F);

template <typename T>
using first_argument = decltype(first_argument_helper(std::declval<T>()));


using map_fn = std::function<std::any(std::any)>;

// Trait to get the argument type of a unary function
template<typename T>
struct unary_func_arg_pointer;

template<typename R, typename T>
struct unary_func_arg_pointer< R(*)(T*) >
{
    using type = T;
};

// Trait to get the argument type of a unary function
template<typename T>
struct unary_func_arg;

template<typename R, typename T>
struct unary_func_arg< R(*)(T) >
{
    using type = T;
};

template<typename Fn>
map_fn wrap(Fn&& fn) {
    using Arg = typename unary_func_arg<Fn>::type;
    return [&](const std::any& x) {
        Arg v = std::get<Arg>(x);
        return std::invoke<Fn>(fn, v);
    };
}

template<typename Arg>
void test_fn(Arg a) {}



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
    auto n_1 = create_node(fn_1);

    n_1.Invoke("123");

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

    auto xn_4 = create_node(fn_4, "fn_4");
    auto xn_5 = create_node(fn_5, "fn_5");
    using T = decltype(fn_4);
    JoinFunctionsGraphNode<T> map_node;
    map_node.AddPort("fn_4", fn_4);
    map_node.AddPort("fn_5", fn_5);





    // (fn_4 + fn_4) |

    // struct T {std::string a, std::vector<Doc> docs;}
    // create_map({"x_2": xn_1, "x_2": xn_2}) -> map_result
    // combie






}