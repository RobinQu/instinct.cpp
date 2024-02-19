//
// Created by RobinQu on 2024/2/16.
//

#ifndef RUNNABLELAMBDA_H
#define RUNNABLELAMBDA_H
#include "CoreGlobals.h"
#include "Runnable.h"

namespace LC_CORE_NS {


template<typename Input, typename Output>
class RunnableLambda: public Runnable<Input, Output> {

    std::function<Output(Input)> fn_;

public:
    RunnableLambda()=delete;
    explicit RunnableLambda(std::function<Output(Input)> fn)
        : fn_(std::move(fn)) {
    }

    Output Invoke(const Input& input) override;

    std::initializer_list<Output> Batch(const std::initializer_list<Input>& input_range) override;

    std::initializer_list<Output> Stream(const Input& input) override;
};

} // LC_CORE_NS

#endif //RUNNABLELAMBDA_H
