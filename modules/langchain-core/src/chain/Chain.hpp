//
// Created by RobinQu on 2024/2/20.
//

#ifndef CHAINABLE_HPP
#define CHAINABLE_HPP

#include <rpp/rpp.hpp>

#include "CoreGlobals.hpp"
#include "tools/ChunkStreamView.hpp"
#include "tools/ResultIterator.hpp"

LC_CORE_NS {
    // template<typename T>
    // concept RuntimeOptionsALike =
    //         requires(T t)
    //         {
    //             {
    //                 T::Defaults()
    //             } -> std::same_as<T>;
    //         } ;

    template<
        typename Configuration,
        typename RuntimeOptions,
        typename Input,
        typename Output,
    typename OutputChunk=Output,
    typename OutputChunkRange=ResultIterator<OutputChunk> *,
    typename OutputRange=ResultIterator<Output>*,
    typename InputRange=ResultIterator<Input>*
>
        requires RangeOf<OutputRange, Output> && RangeOf<OutputChunkRange, OutputChunk> && RangeOf<InputRange, Input>
    class Chain {
    public:
        Chain() = default;

        explicit Chain(Configuration configuration);

        Chain(Chain&&) = delete;

        Chain(const Chain&) = delete;

        virtual ~Chain() = default;

        virtual Output Invoke(const Input& input, const RuntimeOptions& options) = 0;

        virtual OutputRange Batch(const InputRange& input, const RuntimeOptions& options) = 0;

        virtual OutputChunkRange Stream(const Input& input, const RuntimeOptions& options) = 0;

        virtual Output Invoke(const Input& input) = 0;

        virtual OutputRange Batch(const InputRange& input) =0;

        virtual OutputChunkRange Stream(const Input& input) =0;

        virtual Output operator()(const Input& input) { return Invoke(input, RuntimeOptions::Defaults()); }
    };
}


#endif //CHAINABLE_HPP
