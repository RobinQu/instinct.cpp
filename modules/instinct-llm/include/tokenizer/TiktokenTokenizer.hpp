//
// Created by RobinQu on 2024/3/1.
//

#ifndef TIKTOKENTOKENIZER_HPP
#define TIKTOKENTOKENIZER_HPP

#include <filesystem>
#include "RegexTokenizer.hpp"
#include <ranges>

#include "GPT2BPEFileReader.hpp"
#include "TiktokenBPEFileReader.hpp"
#include "tools/file_vault/FileSystemFileVault.hpp"


namespace INSTINCT_LLM_NS {

    using ByteShuffle = std::unordered_map<u_int8_t, u_int8_t>;
    // using ReversedByteShuffle = std::unordered_map<int32_t, u_int8_t>;

    static void PreloadTokenizerResources(const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
        if(!DEFAULT_FILE_VAULT->CheckResource("tiktoken/gpt2_vocab.bpe").get()) {
            FetchHttpGetResourceToFileVault(
                DEFAULT_FILE_VAULT,
                "tiktoken/gpt2_vocab.bpe",
                "https://openaipublic.blob.core.windows.net/gpt-2/encodings/main/vocab.bpe",
                {.algorithm = kSHA256, .expected_value = "1ce1664773c50f3e0cc8842619a93edc4624525b728b188a9e0be33b7726adc5"}
            ).wait();
        }

        if(!DEFAULT_FILE_VAULT->CheckResource("tiktoken/gpt2_encoder.json").get()) {
            FetchHttpGetResourceToFileVault(
                DEFAULT_FILE_VAULT,
                "tiktoken/gpt2_encoder.json",
                "https://openaipublic.blob.core.windows.net/gpt-2/encodings/main/encoder.json",
                {.algorithm = kSHA256, .expected_value = "196139668be63f3b5d6574427317ae82f612a97c5d1cdaf36ed2256dbf636783"}
            ).wait();
        }
        if(!DEFAULT_FILE_VAULT->CheckResource("tiktoken/cl100k_base.tiktoken").get()) {
            FetchHttpGetResourceToFileVault(
                DEFAULT_FILE_VAULT,
                "tiktoken/cl100k_base.tiktoken",
                "https://openaipublic.blob.core.windows.net/encodings/cl100k_base.tiktoken",
                {.algorithm = kSHA256, .expected_value = "223921b76ee99bde995b7ff738513eef100fb51d18c93597a113bcffe865b2a7"}
            ).wait();
        }
    }

    static TokenizerPtr GPT4_TOKENIZER_INSTANCE = nullptr;
    static TokenizerPtr GPT2_TOKENIZER_INSTANCE = nullptr;
    static std::mutex MUTEX;


    struct TiktokenConfig {
        std::string name;
        int explict_n_vocab;
        std::string pat_str;
        BPETokenRanks mergeable_ranks;
        StringIDDict special_tokens;
    };

    class TiktokenTokenizer final: public RegexTokenizer {
        ByteShuffle byte_shuffle_;
        ByteShuffle reversed_byte_shuffle_;

    public:
        TiktokenTokenizer(BPERanks bpe_ranks, Vocab vocab, const UnicodeString& regexp_string,
            const StringIDDict& special_tokens, ByteShuffle byte_shuffle)
            : RegexTokenizer(std::move(bpe_ranks), std::move(vocab), regexp_string, special_tokens),
              byte_shuffle_(std::move(byte_shuffle)) {
            for(const auto& [id,token]: byte_shuffle_) {
                reversed_byte_shuffle_[token] = id;
            }
        }

        void Train(const UnicodeString& text, int vocab_size) override {
            throw InstinctException("Not implemented");
        }

        static TokenizerPtr FromTiktokenConfig(const TiktokenConfig& config) {
            BPERanks bpe_ranks = details::recover_byte_pair_bpe_ranks(config.mergeable_ranks);

            // init vocab
            Vocab vocab;
            for(int i: std::ranges::iota_view {0, 256}) {
                vocab[i] = Bytes{static_cast<char>(i)};
            }
            for(const auto& [pair, id]: bpe_ranks) {
                // rebuild vocab from lower rank to higher rank, assuming id > pair.first and id > pair.second
                vocab[id] = vocab[pair.first] + vocab[pair.second];
            }

            // byte mapping
            ByteShuffle byte_shuffle;

            for(const auto i: std::ranges::iota_view {0, 256}) {
                // uint8_t to char, that's from [0.256) to [-128,128)
                const int32_t id = config.mergeable_ranks.at(Bytes{static_cast<char>(i)});
                // first 256 char have rank of less than 256,  so it's safe to cast back to uint8_t
                byte_shuffle[i] = id;
            }
            return std::make_shared<TiktokenTokenizer>(bpe_ranks, vocab, UnicodeString::fromUTF8(config.pat_str), config.special_tokens, byte_shuffle);
        }

        static TokenizerPtr MakeGPT2Tokenizer(const FileVaultPtr& file_vault = DEFAULT_FILE_VAULT) {
            trace_span span {"MakeGPT2Tokenizer"};
            std::lock_guard guard {MUTEX};
            if (!GPT2_TOKENIZER_INSTANCE) {
                PreloadTokenizerResources(file_vault);
                const auto entry1 = DEFAULT_FILE_VAULT->GetResource("tiktoken/gpt2_vocab.bpe").get();
                const auto entry2 = DEFAULT_FILE_VAULT->GetResource("tiktoken/gpt2_vocab.bpe").get();
                GPT2_TOKENIZER_INSTANCE = MakeGPT2Tokenizer(entry1.local_path, entry2.local_path);
            }
            return GPT2_TOKENIZER_INSTANCE;
        }

        static TokenizerPtr MakeGPT2Tokenizer(
            const std::filesystem::path& bpe_file_path,
            const std::filesystem::path& encoder_json_file_path
            ) {
            auto reader = GPT2BPEFileReader(bpe_file_path, encoder_json_file_path);
            return FromTiktokenConfig({
                .name = "gpt2",
                .explict_n_vocab = 50257,
                .pat_str = R"""('(?:[sdmt]|ll|ve|re)| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+)""",
                .mergeable_ranks =  reader.Fetch(),
                .special_tokens = {
                    {"<|endoftext|>", 50256}
                }
            });
        }

        static TokenizerPtr MakeGPT4Tokenizer() {
            trace_span span {"MakeGPT4Tokenizer"};
            std::lock_guard guard {MUTEX};
            if (!GPT4_TOKENIZER_INSTANCE) {
                PreloadTokenizerResources(DEFAULT_FILE_VAULT);
                const auto entry = DEFAULT_FILE_VAULT->GetResource("tiktoken/cl100k_base.tiktoken").get();
                GPT4_TOKENIZER_INSTANCE = MakeGPT4Tokenizer(entry.local_path);
            }
            return GPT4_TOKENIZER_INSTANCE;
        }

        static TokenizerPtr MakeGPT4Tokenizer(
            const std::filesystem::path& tiktoken_bpe_file_path
            ) {
            TiktokenBPEFileReader reader(tiktoken_bpe_file_path);
            return FromTiktokenConfig({
                .name = "cl100k_base",
                .pat_str = R"""('(?i:[sdmt]|ll|ve|re)|[^\r\n\p{L}\p{N}]?+\p{L}+|\p{N}{1,3}| ?[^\s\p{L}\p{N}]++[\r\n]*|\s*[\r\n]|\s+(?!\S)|\s+)""",
                .mergeable_ranks = reader.Fetch(),
                .special_tokens = {
                    {"<|endoftext|>", 100257},
                    {"<|fim_prefix|>", 100258},
                    {"<|fim_middle|>", 100259},
                    {"<|fim_suffix|>", 100260},
                    {"<|endofprompt|>", 100276}
                }
            });
        }

        UnicodeString Decode(const std::vector<int32_t>& ids) override {
            Bytes text_bytes;
            for(const auto& id: ids) {
                text_bytes += GetVocab().at(id);
            }
            Bytes result;
            for(const auto& c: text_bytes) {
                result += static_cast<char>(reversed_byte_shuffle_.at(static_cast<u_int8_t>(c)));
            }
            return UnicodeString::fromUTF8(result);
        }

    private:
        void HandleChunkBytes_(Bytes& text_bytes) override {
            Bytes new_bytes;
            for (const auto&c: text_bytes) {
                new_bytes += static_cast<char>(byte_shuffle_.at(static_cast<u_int8_t>(c)));
            }
            text_bytes = new_bytes;
        }
    };



}




#endif //TIKTOKENTOKENIZER_HPP
