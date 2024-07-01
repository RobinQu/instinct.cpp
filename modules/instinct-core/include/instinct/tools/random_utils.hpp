//
// Created by RobinQu on 2024/4/29.
//

#ifndef RANDOMUTILS_HPP
#define RANDOMUTILS_HPP

#include <random>

#include <instinct/core_global.hpp>

namespace INSTINCT_CORE_NS {
    class RandomUtils final {
    public:
        template<typename T = float>
        static T GetRandom(const T range_start = 0, const T rand_end = 1) {
            std::random_device rd;  // Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
            std::uniform_real_distribution<T> dis(range_start, rand_end);
            return dis(gen);
        }

    };
}


#endif //RANDOMUTILS_HPP
