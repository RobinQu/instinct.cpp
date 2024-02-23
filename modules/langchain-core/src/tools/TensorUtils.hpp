//
// Created by RobinQu on 2024/2/23.
//

#ifndef TENSORUTILS_HPP
#define TENSORUTILS_HPP
#include "CoreGlobals.hpp"
#include "model/BaseEmbeddingModel.hpp"
#include <iostream>

LC_CORE_NS {
    struct TensorUtils final {


        static void PrintEmbedding(const Embedding& embedding, std::ostream& stream = std::cout, const bool flush = true) {
            for(const auto& f: embedding) {
                stream << f;
            }
            if(flush) {
                stream << std::endl;
            }
        }


    };
}



#endif //TENSORUTILS_HPP
