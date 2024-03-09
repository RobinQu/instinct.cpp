//
// Created by RobinQu on 2024/2/20.
//

#ifndef CHAINABLE_HPP
#define CHAINABLE_HPP

#include "CoreGlobals.hpp"
#include "tools/ResultIterator.hpp"

namespace INSTINCT_CORE_NS {

    template<
        typename Input,
        typename Output,
    typename OutputChunk=Output,
    typename OutputChunkRangePtr=ResultIteratorPtr<OutputChunk>,
    typename OutputRangePtr=ResultIteratorPtr<Output>,
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
        virtual OutputRangePtr Batch(const InputRange& input) =0;
        virtual OutputChunkRangePtr Stream(const Input& input) =0;
        virtual Output operator()(const Input& input) { return Invoke(input); }
    };
}


#endif //CHAINABLE_HPP
