//
// Created by RobinQu on 2024/3/3.
//
#include <gtest/gtest.h>


#include "CoreGlobals.hpp"
#include "document/tokenizer/TiktokenBPEFileReader.hpp"

namespace INSTINCT_CORE_NS {
    TEST(BPETokenRanksFileReader, TestLoad) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        TiktokenBPEFileReader reader (assets_dir / "cl100k_base/cl100k_base.tiktoken");
        auto bpe_ranks = reader.Fetch();
        std::cout << "item_cout=" << bpe_ranks.size() << std::endl;
        ASSERT_EQ(bpe_ranks.size(), 100256);
     }
}


