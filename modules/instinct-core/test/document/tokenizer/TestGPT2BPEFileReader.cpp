//
// Created by RobinQu on 2024/3/3.
//

#include <gtest/gtest.h>
#include "CoreGlobals.hpp"
#include "document/tokenizer/GPT2BPEFileReader.hpp"

namespace INSTINCT_CORE_NS {
    TEST(GPT2BPEFileReader, TestLoad) {
        const std::filesystem::path assets_dir = std::filesystem::current_path() / "./modules/instinct-core/test/_assets";
        GPT2BPEFileReader file_reader(
            assets_dir / "gpt2/vocab.bpe",
            assets_dir / "gpt2/encoder.json"
            );
        auto bpe_ranks = file_reader.Fetch();
        std::cout <<"item_count=" <<  bpe_ranks.size() << std::endl;
        ASSERT_EQ(bpe_ranks.size(), 50256);
    }
}