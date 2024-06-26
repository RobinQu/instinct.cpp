//
// Created by RobinQu on 3/23/24.
//

#ifndef INSTINCT_ICONFIGURABLE_HPP
#define INSTINCT_ICONFIGURABLE_HPP

#include <instinct/CoreGlobals.hpp>

namespace INSTINCT_CORE_NS {
    template<typename Options>
    class IConfigurable {
    public:
        virtual ~IConfigurable()=default;
        virtual void Configure(const Options& options) = 0;
    };
}


#endif //INSTINCT_ICONFIGURABLE_HPP
