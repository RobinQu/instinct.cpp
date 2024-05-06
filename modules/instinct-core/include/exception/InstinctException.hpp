//
// Created by RobinQu on 2024/5/4.
//

#ifndef INSTINCTEXCEPTION_HPP
#define INSTINCTEXCEPTION_HPP
#include <cpptrace/cpptrace.hpp>
#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    class InstinctException: public cpptrace::exception_with_message {
    public:
        explicit InstinctException(std::string reason)
            : cpptrace::exception_with_message(std::move(reason)) {
        }
    };
}


#endif //INSTINCTEXCEPTION_HPP
