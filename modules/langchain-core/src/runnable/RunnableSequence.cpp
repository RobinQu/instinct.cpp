//
// Created by RobinQu on 2024/2/16.
//

#include "RunnableSequence.h"

namespace LC_CORE_NS {
    template <class F, class... Fs>
    auto composer(F&& arg, Fs&&... args)
    {
        return [fun = std::forward<F>(arg),
                ...functions = std::forward<Fs>(args)]<class X>(X&& x) mutable {
                    if constexpr (sizeof...(Fs))
                    {
                        return composer(std::forward<Fs>(functions)...)(
                            std::invoke(std::forward<F>(fun), std::forward<X>(x)));
                    }
                    else
                    {
                        return std::invoke(std::forward<F>(fun), std::forward<X>(x));
                    }
                };
    }



} // LC_CORE_NS