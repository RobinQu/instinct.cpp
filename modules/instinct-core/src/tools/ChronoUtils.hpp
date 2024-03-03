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
            auto currentTime = std::chrono::system_clock::now();
            char buffer[80];
            auto transformed = currentTime.time_since_epoch().count() / 1000000;
            auto millis = transformed % 1000;
            std::time_t tt;
            tt = std::chrono::system_clock::to_time_t ( currentTime );
            auto timeinfo = localtime (&tt);
            strftime (buffer,80,"%F %H:%M:%S",timeinfo);
            sprintf(buffer, "%s:%03d",buffer,(int)millis);
            return {buffer};
        }

    };

}




#endif //CHORNOUTILS_HPP
