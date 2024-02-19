//
// Created by RobinQu on 2024/2/16.
//

#ifndef RUNNABLESEQUENCE_H
#define RUNNABLESEQUENCE_H

#include "CoreGlobals.h"
#include "Runnable.h"

namespace LC_CORE_NS {

template<typename I, typename M, typename  R>
class RunnableSequence: public Runnable<I, R> {
    using F1 = std::function<M(I)>;
    using F2 = std::function<R(M)>;
    F1 fn1_;
    F2 fn2_;
public:
    RunnableSequence() = delete;

    RunnableSequence(F1&& f1, F2&& f2);

    R Invoke(const I& input) override;

    std::initializer_list<R> Batch(const std::initializer_list<I>& input_range) override;

    std::initializer_list<R> Stream(const I& input) override;
};

} // LC_CORE_NS

#endif //RUNNABLESEQUENCE_H
