//
// Created by RobinQu on 2024/2/29.
//

#ifndef GPT4TOKENIZER_HPP
#define GPT4TOKENIZER_HPP

#include "CoreGlobals.hpp"
#include "PretrainedTokenizer.hpp"
#include <ranges>
#include <unordered_set>

#include "tools/StringUtils.hpp"


namespace INSTINCT_CORE_NS {
    enum AllowSpecialType {
        kUnspecified,
        kAll,
        kNone,
        kSome,
        kNoneRaise
    };

    struct RegexpEncodeOptions {
        AllowSpecialType allow_special = kNoneRaise;
        std::vector<UnicodeString> specials = {};
    };

    class RegexpTokenizer: public PretrainedTokenizer {
        BPERanks merges_;
        Vocab vocab_;
        UnicodeString regexp_pattern_;
        RevseredVocab special_tokens_;
        Vocab reversed_special_tokens_;

    public:
        // RegexpTokenizer()=delete;

        RegexpTokenizer(BPERanks bpe_ranks, Vocab vocab, const UnicodeString& regexp_string, RevseredVocab special_tokens):  merges_(std::move(bpe_ranks)), vocab_(std::move(vocab)), regexp_pattern_(regexp_string), special_tokens_(std::move(special_tokens)), reversed_special_tokens_() {
            // initialize revsered speicial tokens
            for (const auto&[token,id]: special_tokens_) {
                reversed_special_tokens_[id] = token;
            }
        }

        std::string Decode(const std::vector<int32_t>& ids) override {
            std::string buf;
            return Decode_(ids).toUTF8String(buf);
        }

        std::vector<int32_t> Encode(const std::string& text) override {
            const UnicodeString text_u32 = details::conv_to_unicode_string(text);
            return Encode_(text_u32, {});
        }


    private:

        UnicodeString Decode_(const std::vector<int32_t>& ids) {
            UnicodeString result;
            for(const auto& id: ids) {
                if (vocab_.contains(id)) {
                    result.append(vocab_[id]);
                } else if(reversed_special_tokens_.contains(id)) {
                    result.append(reversed_special_tokens_[id]);
                } else {
                    throw LangchainException("Invalid token value found: " + std::to_string(id));
                }
            }
            return result;
        }

        std::vector<int32_t> Encode_(const UnicodeString& text, const RegexpEncodeOptions& options) {
            RevseredVocab specials;
            switch (options.allow_special) {
                case kAll:
                    auto view = special_tokens_ | std::views::keys;
                    for(const auto&token : special_tokens_ | std::views::keys) {
                        specials.emplace(token, special_tokens_[token]);
                    }
                    break;
                case kNone:
                    break;
                case kNoneRaise:
                    for(const auto& token: special_tokens_ | std::views::keys) {
                        if(text.indexOf(token)) {
                            throw LangchainException("special token not allowed in text");
                        }
                    }
                    break;
                case kSome:
                    for(const auto& token: options.specials) {
                        specials.emplace(token, special_tokens_[token]);
                    }
                case kUnspecified:
                    throw LangchainException("allowd_special unspecified");
            }


            if(specials.empty()) {
                return EncodeOrdinary_(text);
            }

            const auto pattern_string = "(" + details::join_with_seperator("|", specials) + ")";
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

        std::vector<int32_t> EncodeOrdinary_(const UnicodeString& text) {
            std::vector<int32_t> result;
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher regex_matcher_(regexp_pattern_, 0, status);
            if(U_FAILURE(status)) {
                throw LangchainException("Failed to create RegexMatcher before encoding");
            }
            regex_matcher_.reset(text);
            while (regex_matcher_.find()) {
                const int start = regex_matcher_.start(status);
                if(U_FAILURE(status)) {
                    throw LangchainException("Failed to match start during encoding");
                }
                int end = std::min(regex_matcher_.end(status), text.length());
                if(U_FAILURE(status)) {
                    throw LangchainException("Failed to match end during encoding");
                }
                UnicodeString chunk = text.tempSubStringBetween(start, end);
                Bytes buf;
                auto parts = EncodeChunk_(chunk.toUTF8String(buf));
                result.insert(result.end(), parts.begin(), parts.end());
            }
            return result;
        }

        std::vector<int32_t> EncodeChunk_(const Bytes& text_bytes) {
            std::vector<int32_t> ids;
            for(const auto &c: text_bytes) {
                ids.push_back(c);
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
