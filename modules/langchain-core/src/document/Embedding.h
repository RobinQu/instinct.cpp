//
// Created by RobinQu on 2024/1/13.
//

#ifndef EMBEDDING_H
#define EMBEDDING_H

#include <xtensor/xarray.hpp>

namespace langchain {
namespace core {


struct Embedding {
    xt::xarray<float> tensor;
    int dimension;
};
using EmbeddingPtr = std::shared_ptr<Embedding>;

} // core
} // langchain

#endif //EMBEDDING_H
