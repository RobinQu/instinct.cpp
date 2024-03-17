//
// Created by RobinQu on 2024/3/2.
//

#ifndef TIKTOKENBPEFILEREADER_HPP
#define TIKTOKENBPEFILEREADER_HPP

#include "BPETokenRanksReader.hpp"
#include <fstream>

#include "tools/StringUtils.hpp"
#include <base64.hpp>

namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    class TiktokenBPEFileReader: public BPETokenRanksReader {
        std::filesystem::path bpe_file_path_;

    public:
        explicit TiktokenBPEFileReader(std::filesystem::path bpe_file_path)
            : bpe_file_path_(std::move(bpe_file_path)) {
        }
        BPETokenRanks Fetch() override;
    };

    inline BPETokenRanks TiktokenBPEFileReader::Fetch() {
        std::ifstream bpe_file(bpe_file_path_);
        if (!bpe_file.is_open()) {
            throw InstinctException("failed to open bpe file at " + bpe_file_path_.string());
        }

        BPETokenRanks bpe_token_ranks;
        for (std::string line; std::getline(bpe_file, line);) {
            // as content is encoded in base64, it's fine to use std::string
            auto splits = StringUtils::ReSplit(line);
            if (splits.size()!=2) {
                // TODO warn about invalid line
                continue;
            }
            auto key = base64::from_base64(splits[0]);
            bpe_token_ranks[key] = std::stoi(splits[1]);
        }

        return bpe_token_ranks;
    }
}

#endif //TIKTOKENBPEFILEREADER_HPP
