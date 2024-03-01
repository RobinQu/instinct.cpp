//
// Created by RobinQu on 2024/3/1.
//

#ifndef TIKTOKENTOKENIZER_HPP
#define TIKTOKENTOKENIZER_HPP

#include <filesystem>
#include "RegexpTokenizer.hpp"
#include <ranges>


namespace INSTINCT_CORE_NS {

    class TiktokenTokenizer: public RegexpTokenizer {
        std::unordered_map<int8_t, int8_t> byte_shuffle_;
        std::unordered_map<int8_t, int8_t> revsered_byte_shuffle_;
    public:
        TiktokenTokenizer(BPETokenRanks bpe_token_ranks, const UnicodeString& regexp_string,
            RevseredVocab special_tokens)
            : RegexpTokenizer(std::move(bpe_ranks), std::move(vocab), regexp_string, std::move(special_tokens)) {


        }

        static TiktokenTokenizer* FromTiktokenBPE(const std::filesystem::path& bpe_file_path) {
            BPETokenRanks bpe_token_ranks;
            BPERanks bpe_ranks = details::recover_byte_pair_bpe_ranks(bpe_token_ranks);
            Vocab vocab;

            for(auto i: std::ranges::iota_view {0, 256}) {
                // byte_shuffle_[i] = merges_[];
                // vocab[i] = int
            }

        }

        static TiktokenTokenizer* FromGPT2BPE(const std::filesystem::path& bpe_file_path, const std::filesystem::path& encoder_json_file_path) {

        }

    private:
        void HandleChunkBytes_(Bytes& text_bytes) override {
            Bytes new_bytes;
            for (const auto&c: text_bytes) {
                new_bytes += byte_shuffle_[c];
            }
            text_bytes = new_bytes;
        }
    };



}




#endif //TIKTOKENTOKENIZER_HPP
