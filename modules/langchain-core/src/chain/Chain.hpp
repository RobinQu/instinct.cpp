//
// Created by RobinQu on 2024/2/20.
//

#ifndef CHAINABLE_HPP
#define CHAINABLE_HPP

#include <rpp/rpp.hpp>

#include "CoreGlobals.hpp"

LC_CORE_NS {

    template <typename T>
        concept RuntimeOptionsALike =
            requires(T t){
                {
                    T::Defaults()
                } -> std::same_as<T>;
            } ;

    template <
        typename Input,
        typename Output,
        typename RuntimeOptions,
        typename OutputChunk=Output,
        typename InputRange=std::vector<Input>,
        typename OutputRange=std::vector<Output>,
        typename OutputChunkRange=std::vector<OutputChunk>
    >
    requires RangeOf<OutputRange, Output> && RangeOf<OutputChunkRange, OutputChunk> && RangeOf<InputRange, Input> && RuntimeOptionsALike<RuntimeOptions>
    class Chain {
    public:
        Chain()=default;
        Chain(Chain&&)=delete;
        Chain(const Chain&)=delete;
        virtual ~Chain()=default;
        virtual Output Invoke(const Input& input, const RuntimeOptions& options) = 0;
        virtual OutputRange Batch(const InputRange& input, const RuntimeOptions& options) = 0;
        virtual OutputChunkRange Stream(const Input& input, const RuntimeOptions& options) = 0;

        // virtual Output Invoke(const Input& input) = 0;
        // virtual OutputRange Batch(const InputRange& input)=0;
        // virtual OutputChunkRange Stream(const Input& input)=0;
        virtual Output operator()(const Input& input) {return Invoke(input, RuntimeOptions::Defaults());}
    };



}



#endif //CHAINABLE_HPP
