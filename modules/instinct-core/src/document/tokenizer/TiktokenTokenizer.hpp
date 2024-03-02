//
// Created by RobinQu on 2024/3/1.
//

#ifndef TIKTOKENTOKENIZER_HPP
#define TIKTOKENTOKENIZER_HPP

#include <filesystem>
#include "RegexpTokenizer.hpp"
#include <ranges>


namespace INSTINCT_CORE_NS {

    using ByteShuffle = std::unordered_map<int8_t, int8_t>;

    struct TiktokenConfig {
        std::string name;
        int explict_n_vocab;
        std::string pat_str;
        BPETokenRanks mergeable_ranks;
        StringIDDict special_tokens;
    };

    class TiktokenTokenizer: public RegexpTokenizer {
        ByteShuffle byte_shuffle_;
        ByteShuffle revsered_byte_shuffle_;

    public:
        TiktokenTokenizer(BPERanks bpe_ranks, Vocab vocab, const UnicodeString& regexp_string,
            const StringIDDict& special_tokens, ByteShuffle byte_shuffle)
            : RegexpTokenizer(std::move(bpe_ranks), std::move(vocab), regexp_string, special_tokens),
              byte_shuffle_(std::move(byte_shuffle)) {
            for(const auto& [id,token]: byte_shuffle_) {
                revsered_byte_shuffle_[token] = id;
            }
        }

        static TiktokenTokenizer* FromTiktokenConfig(const TiktokenConfig& config) {
            BPERanks bpe_ranks = details::recover_byte_pair_bpe_ranks(config.mergeable_ranks);

            // init vocab
            Vocab vocab;
            for(const auto i: std::ranges::iota_view {0, 256}) {
                vocab[i] = Bytes{static_cast<char>(i)};
            }
            for(const auto& [pair, id]: bpe_ranks) {
                // rebuild vocab from from lower rank to higher rank, assuming id > pair.first and id > pair.second
                vocab[id] = vocab[pair.first] + vocab[pair.second];
            }

            // byte mapping
            ByteShuffle byte_shuffle;
            for(const auto i: std::ranges::iota_view {0, 256}) {
                // NOTE: uint8_t to char, that's from [0.256) to [-128,127)
                const int8_t id = config.mergeable_ranks.at(Bytes{static_cast<char>(i)});
                byte_shuffle[i] = id;
            }
            return new TiktokenTokenizer(bpe_ranks, vocab, UnicodeString::fromUTF8(config.pat_str), config.special_tokens, byte_shuffle);
        }

        static TiktokenTokenizer* MakeGPT2Tokenizer(
            const std::filesystem::path& bpe_file_path,
            const std::filesystem::path& encoder_json_file_path) {

        }

        static TiktokenTokenizer* MakeGPT4Tokenizer(
            const std::filesystem::path& tiktoken_bpe_file_path
            ) {

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
