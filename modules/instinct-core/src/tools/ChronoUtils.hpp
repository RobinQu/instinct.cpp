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
            auto current_time = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count();
        }

        static std::string GetCurrentTimestampString()
        {
            return std::to_string(GetCurrentTimeMillis());
        }

    };

}




#endif //CHORNOUTILS_HPP
