//
// Created by RobinQu on 2024/3/3.
//

#ifndef CHORNOUTILS_HPP
#define CHORNOUTILS_HPP

#include <chrono>
#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {

    class ChronoUtils {
    public:

        static long GetCurrentTimeMillis() {
            const auto current_time = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count();
        }

        static long GetCurrentEpochSeconds() {
            const auto current_time = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::seconds>(current_time.time_since_epoch()).count();
        }

        static long GetCurrentEpochMicroSeconds() {
            const auto current_time = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(current_time.time_since_epoch()).count();
        }

        template<typename TimeUnit>
        static long GetLaterEpoch(const std::chrono::time_point<std::chrono::system_clock>::duration& duration_from_now) {
            const auto current_time = std::chrono::system_clock::now();
            return std::chrono::duration_cast<TimeUnit>((current_time+duration_from_now).time_since_epoch()).count();
        }

        static std::string GetCurrentTimestampString()
        {
            return std::to_string(GetCurrentTimeMillis());
        }

    };

}




#endif //CHORNOUTILS_HPP
