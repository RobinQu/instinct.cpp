//
// Created by RobinQu on 2024/2/20.
//

#ifndef CHAINABLE_HPP
#define CHAINABLE_HPP

#include <instinct/CoreGlobals.hpp>

#include <instinct/functional/ReactiveFunctions.hpp>

namespace INSTINCT_CORE_NS {
    template<
        typename Input,
        typename Output,
    typename OutputChunk=Output,
    typename OutputChunkIterator=AsyncIterator<OutputChunk>,
    typename OutputIterator=AsyncIterator<Output>,
    typename InputRange=std::vector<Input>
>
    requires RangeOf<InputRange, Input>
    class IRunnable {
    public:
        IRunnable() = default;
        IRunnable(IRunnable&&) = delete;
        IRunnable(const IRunnable&) = delete;
        virtual ~IRunnable() = default;
        virtual Output Invoke(const Input& input) = 0;
        virtual OutputIterator Batch(const InputRange& input) =0;
        virtual OutputChunkIterator Stream(const Input& input) =0;
        virtual Output operator()(const Input& input) { return Invoke(input); }
    };


}


#endif //CHAINABLE_HPP
