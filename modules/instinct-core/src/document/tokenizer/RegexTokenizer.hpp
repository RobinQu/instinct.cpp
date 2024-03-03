//
// Created by RobinQu on 2024/2/29.
//

#ifndef GPT4TOKENIZER_HPP
#define GPT4TOKENIZER_HPP

#include "CoreGlobals.hpp"
#include "Tokenizer.hpp"
#include <ranges>
#include <unordered_set>
#include <utility>

#include "tools/StringUtils.hpp"


namespace INSTINCT_CORE_NS {
    enum AllowSpecialType {
        kUnspecified,
        kAll,
        kNone,
        kSome,
        kNoneRaise
    };

    struct RegexEncodeOptions {
        AllowSpecialType allow_special = kNoneRaise;
        std::vector<UnicodeString> specials = {};
    };

    class RegexTokenizer: public Tokenizer {
        BPERanks merges_;
        Vocab vocab_{};
        UnicodeString regexp_pattern_{};
        StringIDDict special_tokens_{};
        ReversedStringIDDict reversed_special_tokens_{};

    public:
        RegexTokenizer()=delete;
        RegexTokenizer(UnicodeString regexp_string, const StringIDDict& special_tokens): regexp_pattern_(std::move(regexp_string)) {
            RegisterSpecials(special_tokens);
        }
        RegexTokenizer(BPERanks bpe_ranks, Vocab vocab, UnicodeString  regexp_string, const StringIDDict& special_tokens):  merges_(std::move(bpe_ranks)), vocab_(std::move(vocab)), regexp_pattern_(std::move(regexp_string)) {
            // initialize revsered speicial tokens
            RegisterSpecials(special_tokens);
        }

        void RegisterSpecials(const StringIDDict& special_tokens) {
            for (const auto&[token,id]: special_tokens) {
                special_tokens_[token] = id;
                reversed_special_tokens_[id] = token;
            }
        }

        Vocab& GetVocab() {
            return vocab_;
        }

        UnicodeString Decode(const std::vector<int32_t>& ids) override {
            return UnicodeString::fromUTF8(Decode_(ids));
        }

        std::vector<int32_t> Encode(const UnicodeString& text) override {
            return Encode(text, {});
        }

        std::vector<int32_t> Encode(const UnicodeString& text, const RegexEncodeOptions& options) {
            StringIDDict specials;
            switch (options.allow_special) {
                case kAll:
                    for(const auto&token : special_tokens_ | std::views::keys) {
                        specials.emplace(token, special_tokens_[token]);
                    }
                break;
                case kNone:
                    break;
                case kNoneRaise:
                    for(const auto& token: special_tokens_ | std::views::keys) {
                        if(text.indexOf(token)) {
                            throw InstinctException("special token not allowed in text");
                        }
                    }
                break;
                case kSome:
                    for(const auto& token: options.specials) {
                        specials.emplace(token, special_tokens_[token]);
                    }
                case kUnspecified:
                    throw InstinctException("allowd_special unspecified");
            }
            if(specials.empty()) {
                return EncodeOrdinary_(text);
            }
            const auto pattern_string = "(" + details::join_with_seperator(
                "|",
                specials | std::views::keys | std::views::transform([](const auto& s) {return details::escape_for_regular_expression(s);})
                ) + ")";


            std::vector<UnicodeString> chunks;
            details::split_text_with_regex(text, pattern_string, chunks);
            std::vector<int32_t> result;
            for(const auto& chunk: chunks) {
                if(specials.contains(chunk)) {
                    result.push_back(specials[chunk]);
                } else {
                    auto chunk_result = EncodeOrdinary_(chunk);
                    result.insert(result.end(), chunk_result.begin(), chunk_result.end());
                }
            }
            return result;
        }

        void Train(const UnicodeString& text, int vocab_size) override {
            if (vocab_size<256) {
                throw InstinctException("vocab_size should be greter than 256");
            }
            int num_merges = vocab_size - 256;
            // Bytes text_bytes;
            // text.toUTF8String(text_bytes);
            // auto ids_view = text_bytes | std::views::transform([](const char c) {
            //     return static_cast<u_int32_t>(c);
            // });
            // auto ids = std::vector{ids_view.begin(), ids_view.end()};
            std::vector<UnicodeString> splits;
            details::find_all_with_regex(text, regexp_pattern_, splits);

            std::vector<std::vector<int32_t>> ids;
            for(const auto& chunk: splits) {
                Bytes buf;
                chunk.toUTF8String(buf);
                auto int_view = buf | std::views::transform([](char c) {
                    return static_cast<u_int8_t>(c);
                });
                ids.emplace_back(int_view.begin(), int_view.end());
            }

            BPERanks merges;
            Vocab vocab;
            for(int i=0;i<256;i++) {
                vocab[i] = Bytes{static_cast<char>(i)};
            }
            for (int i=0;i<num_merges;i++) {
                BPERanks stats;
                for(const auto& chunk_ids: ids) {
                    details::compute_pairs_state(chunk_ids, stats);
                }
                auto pair = details::get_max_pair(stats);
                int32_t idx = 256 + i;

                for(auto& chunk_ids: ids) {
                    // auto copy = chunk_ids;
                    details::merge_u32_ids(chunk_ids, pair, idx);
                }

                merges[pair] = idx;
                vocab[idx] = vocab[pair.first] + vocab[pair.second];
            }
            this->merges_ = std::move(merges);
            this->vocab_ = std::move(vocab);
        }


    private:
        std::string Decode_(const std::vector<int32_t>& ids) {
            std::string text_bytes;
            for(const auto& id: ids) {
                if (vocab_.contains(id)) {
                    text_bytes.append(vocab_[id]);
                } else if(reversed_special_tokens_.contains(id)) {
                    text_bytes.append(details::conv_to_utf8_string(reversed_special_tokens_[id]));
                } else {
                    throw InstinctException("Invalid token value found: " + std::to_string(id));
                }
            }
            return text_bytes;
        }



        std::vector<int32_t> EncodeOrdinary_(const UnicodeString& text) {
            std::vector<int32_t> result;

            std::vector<UnicodeString> text_chunks;
            details::find_all_with_regex(text, regexp_pattern_, text_chunks);

            for(const auto& chunk: text_chunks) {
                Bytes buf;
                chunk.toUTF8String(buf);
                auto parts = EncodeChunk_(buf);
                result.insert(result.end(), parts.begin(), parts.end());
            }
            // UErrorCode status = U_ZERO_ERROR;
            // RegexMatcher regex_matcher_(regexp_pattern_, 0, status);
            // if(U_FAILURE(status)) {
            //     throw InstinctException("Failed to create RegexMatcher before encoding");
            // }
            // regex_matcher_.reset(text);
            // while (regex_matcher_.find()) {
            //     const int start = regex_matcher_.start(status);
            //     if(U_FAILURE(status)) {
            //         throw InstinctException("Failed to match start during encoding");
            //     }
            //     int end = std::min(regex_matcher_.end(status), text.length());
            //     if(U_FAILURE(status)) {
            //         throw InstinctException("Failed to match end during encoding");
            //     }
            //     UnicodeString chunk = text.tempSubStringBetween(start, end);
            //     Bytes buf;
            //     chunk.toUTF8String(buf);
            //     auto parts = EncodeChunk_(buf);
            //     result.insert(result.end(), parts.begin(), parts.end());
            // }
            return result;
        }

        virtual void HandleChunkBytes_(Bytes& text_bytes) {}

        std::vector<int32_t> EncodeChunk_(Bytes& text_bytes) {
            // give subclass changes to alter `text_bytes`
            HandleChunkBytes_(text_bytes);
            std::vector<int32_t> ids;
            for(const auto &c: text_bytes) {
                // from [-128,127) to [0,256)
                ids.push_back(static_cast<u_int8_t>(c));
            }
            while (ids.size()>=2) {
                auto stats = details::compute_pairs_state(ids);
                int32_t min_idx = INT32_MAX;
                auto min_entry = stats.end();
                for(auto itr=stats.begin();itr!=stats.end();++itr) {
                    if(merges_.contains(itr->first)) {
                        min_idx = std::min(merges_[itr->first],min_idx);
                        min_entry = itr;
                    }
                }
                if (min_entry==stats.end()) {// no mergeable found
                    break;
                }
                auto idx = merges_[min_entry->first];
                details::merge_u32_ids(ids, min_entry->first, idx);
            }
            return ids;
        }


    };


}

#endif //GPT4TOKENIZER_HPP
