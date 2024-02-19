//
// Created by RobinQu on 2024/2/16.
//

#ifndef RUNNABLEPARALLEL_H
#define RUNNABLEPARALLEL_H
#include "CoreGlobals.h"
#include "Runnable.h"

namespace LC_CORE_NS {
    template<typename Input, typename Output>
    class RunnableParallel: public Runnable<Input, Output> {
        // RunnableMapping<Input, Output> mapping_;

    public:

        Output Invoke(const Input& input) override;

        std::initializer_list<Output> Batch(const std::initializer_list<Input>& input_range) override;

        std::initializer_list<Output> Stream(const Input& input) override;
    };



} // LC_CORE_NS

#endif //RUNNABLEPARALLEL_H
