//
// Created by RobinQu on 2024/2/25.
//


#include <gtest/gtest.h>
#include "CoreGlobals.hpp"
#include "tools/ChunkStreamView.hpp"


LC_CORE_NS {


    TEST(ChunkStreamView, SimpleTest) {
        std::vector vec1 {1,2,3};
        const auto csv = wrap_chunk_stream_view(vec1);
        for(const auto& v:csv) {
            std::cout<<v<<std::endl;
        }
    }

}

