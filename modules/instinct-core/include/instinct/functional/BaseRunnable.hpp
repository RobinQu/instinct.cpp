//
// Created by RobinQu on 3/21/24.
//

#ifndef INSTINCT_BASERUNNABLE_HPP
#define INSTINCT_BASERUNNABLE_HPP

#include <instinct/CoreGlobals.hpp>
#include <instinct/functional/IRunnable.hpp>
#include <instinct/functional/IConfigurable.hpp>

namespace INSTINCT_CORE_NS {

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
