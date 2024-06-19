//
// Created by RobinQu on 2024/2/23.
//

#ifndef TENSORUTILS_HPP
#define TENSORUTILS_HPP
#include "CoreGlobals.hpp"
#include <iostream>

namespace INSTINCT_CORE_NS {
    struct TensorUtils final {


        static void PrintEmbedding(const std::string& announce, const std::ranges::input_range auto& embedding,  std::ostream& stream = std::cout, const bool flush = true) {
            stream << announce;
            for(const auto& f: embedding) {
                stream << std::setw(14) << std::fixed << std::setprecision(8) << f << ", ";
            }
            if(flush) {
                stream << std::endl;
            }
        }


        static void PrintEmbedding(const std::ranges::input_range auto& embedding, std::ostream& stream = std::cout, const bool flush = true) {
            return PrintEmbedding("", embedding, stream, flush);
        }


    };
}



#endif //TENSORUTILS_HPP
