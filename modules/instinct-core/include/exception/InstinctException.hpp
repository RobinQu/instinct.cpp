//
// Created by RobinQu on 2024/5/4.
//

#ifndef INSTINCTEXCEPTION_HPP
#define INSTINCTEXCEPTION_HPP

#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    class InstinctException: public std::runtime_error {
        std::string reason_;

    public:
        explicit InstinctException(const std::string& basic_string)
            : std::runtime_error(basic_string), reason_(basic_string) {
        }

        explicit InstinctException(const char* const string)
            : std::runtime_error(string), reason_(string) {
        }

        explicit InstinctException(const runtime_error& runtime_error, std::string  basic_string)
            : std::runtime_error(runtime_error),reason_(std::move(basic_string)) {

        }

        [[nodiscard]] const char* what() const noexcept override {
            return reason_.data();
        }
    };
}


#endif //INSTINCTEXCEPTION_HPP
