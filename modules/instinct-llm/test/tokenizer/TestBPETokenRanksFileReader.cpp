//
// Created by RobinQu on 2024/3/3.
//
#include <gtest/gtest.h>


#include <instinct/CoreGlobals.hpp>
#include <instinct/tokenizer/TiktokenBPEFileReader.hpp>
#include <instinct/tools/ChronoUtils.hpp>

namespace INSTINCT_LLM_NS {
    TEST(BPETokenRanksFileReader, TestLoad) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "_assets";
        auto t1 = ChronoUtils::GetCurrentTimeMillis();
        TiktokenBPEFileReader reader (assets_dir / "bpe_ranks/cl100k_base.tiktoken");
        auto bpe_ranks = reader.Fetch();
        std::cout << "bpe loaded in " << ChronoUtils::GetCurrentTimeMillis()-t1 << "ms" << std::endl;
        std::cout << "item_count=" << bpe_ranks.size() << std::endl;
        ASSERT_EQ(bpe_ranks.size(), 100256);
     }
}


