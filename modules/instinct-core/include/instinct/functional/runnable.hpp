//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASERUNNABLE_HPP
#define INSTINCT_BASERUNNABLE_HPP

#include <instinct/core_global.hpp>
#include <instinct/functional/runnable.hpp>

namespace INSTINCT_CORE_NS {

    template<typename Options>
        class IConfigurable {
    public:
        virtual ~IConfigurable()=default;
        virtual void Configure(const Options& options) = 0;
    };

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

    template<typename Input,typename Output>
    class BaseRunnable: public virtual IRunnable<Input,Output> {
    public:
//        Output Invoke(const Input &input) override = 0;

        AsyncIterator<Output> Batch(const std::vector<Input> &input) override {
            return rpp::source::from_iterable(input)
                   | rpp::operators::map([&](const auto& ctx) {
                return this->Invoke(ctx);
            });
        }

        AsyncIterator<Output> Stream(const Input &input) override {
            return rpp::source::just(this->Invoke(input));
        }
    };

    template<
            typename Input,
            typename Output
    >
    using RunnablePtr = std::shared_ptr<BaseRunnable<Input, Output>>;


}




#endif //INSTINCT_BASERUNNABLE_HPP
