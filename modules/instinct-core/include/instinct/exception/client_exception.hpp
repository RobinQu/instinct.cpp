//
// Created by RobinQu on 2024/5/4.
//

#ifndef ASSERTIONEXCEPTION_HPP
#define ASSERTIONEXCEPTION_HPP

#include <instinct/core_global.hpp>
#include <instinct/exception/instinct_exception.hpp>

namespace INSTINCT_CORE_NS {
    /**
     * Represents assertion failure. It has sematic of client error, e.g bad paramter.
     */
    class ClientException final: public InstinctException {
    public:
        explicit ClientException(const std::string &basic_string)
            : InstinctException(basic_string) {
        }
    };
}


#endif //ASSERTIONEXCEPTION_HPP
