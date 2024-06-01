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
         * @param default_value Default to empty string
         * @return Empty string or env value
         */
        static std::string GetEnv(const std::string& name, const std::string& default_value = "") {
            if(const auto v = std::getenv(name.c_str())) {
                return v;
            }
            return default_value;
        }

        static int GetIntEnv(const std::string& name, const int defualt_value = 0) {
            if(const auto v = std::getenv(name.c_str())) {
                return std::stoi(v);
            }
            return defualt_value;
        }

        static unsigned int GetUnsignedIntEnv(const std::string& name, const unsigned int defualt_value = 0) {
            if(const auto v = std::getenv(name.c_str())) {
                const unsigned long lresult = std::stoul(v);
                if (const unsigned int result = lresult; result != lresult) throw std::out_of_range("cannot convert string to unsigned int: " + std::string(v));
            }
            return defualt_value;
        }


        /**
         * Get HOME path for current user
         * @return
         */
        static std::filesystem::path GetHomeDirectory() {
#ifdef _WIN32
            return GetEnv("HOMEDRIVE") + GetEnv("HOMEPATH");
#else
            return GetEnv("HOME");
#endif
        }
    };
}


#endif //SYSTEMUTILS_HPP
