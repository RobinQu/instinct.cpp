//
// Created by RobinQu on 2024/1/13.
//

#ifndef EMBEDDING_H
#define EMBEDDING_H

#include <xtensor/xarray.hpp>
#include <memory>
#include "CoreGlobals.h"
#include <vector>

namespace LC_CORE_NS {

struct Embedding {
    /**
     * \brief usage of xt::array makes it possible to hold abitary shape of tensor data, which may be a single embedding or a group of embeddings
     */
    xt::xarray<float> data;
};
using EmbeddingPtr = std::shared_ptr<Embedding>;

} // core
// langchain

#endif //EMBEDDING_H
