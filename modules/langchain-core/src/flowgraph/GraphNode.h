//
// Created by RobinQu on 2024/2/19.
//

#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include "CoreGlobals.h"
#include "taskflow/taskflow.hpp"

/**
 * \brief  ctx.obserable.subscribe(v -> Invoke(v));
 */
namespace
LC_CORE_NS {
    class AnyMap {
        std::map<std::string, std::any> data_;

    public:

        static AnyMap FromStates(const std::vector<AnyMap>& states) {
        }

        AnyMap() = default;

        template<typename T>
        AnyMap(std::string key, T&& v): data_() {
            Put(key, v);
        }

        template<typename T>
        void Put(const std::string& key, T&& v) {
            data_[key] = std::any(std::forward<T>(v));
        }

        template<typename T>
        T Get(const std::string& k, T&& default_value) {
            if (data_.contains(k)) {
                return std::any_cast<T>(data_[k]);
            }
            return std::forward<T>(default_value);
        }
    };

    // using T = decltype(MappedResult::data_);

    using SS_FN = std::function<AnyMap(const AnyMap&)>;



    struct GraphNode {
        GraphNode() = default;

        virtual ~GraphNode() = default;

        GraphNode(const GraphNode&) = delete;

        GraphNode(GraphNode&&) = delete;

        virtual void Invoke() = 0;

        [[nodiscard]] virtual const std::string& GetName() const = 0;
    };

    class Visitor{};


    template<typename Fn>
    class FunctionGraphNode final : public GraphNode {
        Fn fn_;
        std::string name_;

    public:
        explicit FunctionGraphNode(Fn&& fn, std::string name)
            : fn_(std::forward<Fn>(fn)), name_(std::move(name)) {
        }

        template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
        Output Invoke(const Input& input) {
            return std::invoke<Fn>(fn_, input);
        }

        template<typename Input, typename Output=std::invoke_result_t<Fn, Input>>
        Output operator()(const Input& input) {
            return Invoke(input);
        }

        [[nodiscard]] const std::string& GetName() const override {
            return name_;
        }

        void Visit(Visitor* v) {
            // auto p = GetParam();
            // GetParams().Put("doc_context", {})
            //
            // GetContext().Put("context", {});
            // v->visit(this);
        }

        void Invoke() override {}
    };


    class MyVisistor: public Visitor {

        tf::Taskflow taskflow_;

        template<typename Fn>
        void visit(FunctionGraphNode<Fn> node) {
            taskflow_.emplace([]() {

            });
        }
    };


    // auto chain = prompt | ollama | output_parser
    // struct V {}
    // auto c2 = create_map<V>(combiner, chain1, chain2, chain3);

    //
    // template<typename... Fns, typename Map, typename Xn=std::invoke_result_t<Map>>
    // Xn create_map() {
    //
    // }



    // template <typename Input, typename Intermediate, typename Output, typename Fn1 = Intermediate(Input), typename Fn2 = Output(Intermediate)>
    //     FunctionGraphNode<Input, Output> create_node(Fn1 fn1, Fn2 fn2) {
    //     return FunctionGraphNode<Input,Output>([&](Input& input){
    //         return std::invoke(fn2, std::invoke(fn1, input));
    //     });
    // }





    using MapFunctionGraphNode = FunctionGraphNode<SS_FN>;

    // using Xn = std::function<void(XState&)>;


    template <typename Fn>
    class JoinFunctionsGraphNode : public GraphNode<Fn> {
        std::map<std::string, Fn> fns_;

    public:
        JoinFunctionsGraphNode(): fns_() {}

        // explicit JoinFunctionsGraphNode(std::vector<FunctionGraphNode<Fn>> fns): fns_() {
        //     for(const auto& node: fns) {
        //         fns_[node.GetName()] = node;
        //     }
        // }

        explicit JoinFunctionsGraphNode(std::map<std::string, Fn> fns)
            : fns_(std::move(fns)) {
        }
        //
        // Fn GetRunnable() override {
        //     return [&](const MappedResult& input) {
        //         tf::Taskflow tf;
        //         std::vector<MappedResult> states{fns_.size()};
        //
        //         for (const auto& entry: fns_) {
        //             tf.emplace([&]() {
        //                 states.push_back(std::invoke(entry.second, input));
        //             }).name(entry.first);
        //         }
        //         tf.name("ParallelMap instance");
        //
        //         tf::Executor executor;
        //         executor.run(tf);
        //
        //         return MappedResult::FromStates(states);
        //     };
        // }

        template <typename Input, typename Output = std::invoke_result_t<Fn,Input>, typename JoinFunctionResult = std::map<std::string,Output>>
        JoinFunctionResult Invoke(const Input& input) {
            tf::Taskflow tf;
            JoinFunctionResult result;

            for (const auto& entry: fns_) {
                tf.emplace([&]() {
                    result[entry.first] = std::invoke(entry.second, input);
                }).name(entry.first);
            }
            tf.name("ParallelMap instance");

            tf::Executor executor;
            executor.run(tf);
            return result;
        }


        JoinFunctionsGraphNode& AddPort(const std::string& name, Fn &&fn) {
            fns_[name] = std::forward<Fn>(fn);
            return *this;
        }

        JoinFunctionsGraphNode& AddPort(const std::string& name, const Fn& fn) {
            fns_[name] = fn;
            return *this;
        }

        Fn GetRunnable() override {

        }
    };

    // using MapJoinFunctionsGraphNode = JoinFunctionsGraphNode<


    template<typename Fn>
    FunctionGraphNode<Fn> create_node(Fn&& fn, const std::string& name = "") {
        return FunctionGraphNode<Fn>(std::forward<Fn>(fn,name));
    }



}

#endif //GRAPHNODE_H
