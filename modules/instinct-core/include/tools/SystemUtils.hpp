//
// Created by RobinQu on 2024/4/10.
//

#ifndef SYSTEMUTILS_HPP
#define SYSTEMUTILS_HPP

#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS {
    class SystemUtils final {
    public:
        /**
         * Get env by name.
         * @param name Env name
         * @return Empty string or env value
         */
        static std::string GetEnv(const std::string& name) {
            const auto v = std::getenv(name.c_str());
            if(v) {
                return v;
            }
            return "";
        }
    };
}


#endif //SYSTEMUTILS_HPP
