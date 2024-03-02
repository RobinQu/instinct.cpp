//
// Created by RobinQu on 2024/2/29.
//

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "CoreGlobals.hpp"
#include <map>
#include <utility>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include "CoreTypes.hpp"


namespace INSTINCT_CORE_NS {

    using namespace U_ICU_NAMESPACE;


    struct hash_pair_int32_t {
        size_t operator()(const std::pair<int32_t,int32_t>& p) const {
            auto hash1 = std::hash<int32_t>{}(p.first);
            auto hash2 = std::hash<int32_t>{}(p.second);
            // If hash1 == hash2, their XOR is zero.
            return (hash1 != hash2) ? hash1 ^ hash2 : hash1;
        }
    };

    struct hash_unicode_string {
        size_t operator()(const UnicodeString& s) const {
            return s.hashCode();
        }
    };


    using BPEPair = std::pair<int32_t, int32_t>;
    using BPERanks = std::unordered_map<BPEPair, int32_t, hash_pair_int32_t>;
    using Bytes = std::string;
    using Vocab = std::unordered_map<int32_t, Bytes>;

    // for storing mappings special token to id
    using StringIDDict = std::unordered_map<UnicodeString, int32_t, hash_unicode_string>;
    using ReversedStringIDDict = std::unordered_map<int32_t, UnicodeString>;

    // using BPETokenPair = std::pair<UnicodeString, UnicodeString>;
    using BPETokenRanks = std::unordered_map<Bytes, int32_t>;




    class Tokenizer {
    public:
        // PretrainedTokenizer()=delete;
        Tokenizer()=default;
        virtual ~Tokenizer()=default;
        Tokenizer(Tokenizer&&)=delete;
        Tokenizer(const Tokenizer&)=delete;
        virtual std::vector<int32_t> Encode(const UnicodeString& text) = 0;
        virtual UnicodeString Decode(const std::vector<int32_t>& ids) = 0;
        virtual void Train(const UnicodeString& text, int vocab_size) = 0;

        // virtual size_t GetVocabSize() = 0;
        // virtual std::string IdToToken(int32_t id) = 0;
        // virtual int32_t TokenToId(const std::string& token) = 0;
    };


    /**
     * following details are invisible to you
     */
    namespace details {

        static RegexMatcher* create_regex_matcher(const std::string& pattern) {
            UErrorCode status = U_ZERO_ERROR;
            auto* regex_matcher = new RegexMatcher(UnicodeString::fromUTF8(pattern), 0, status);
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex with pattern string: " + pattern);
            }
            return regex_matcher;
        }

        static void merge_u32_ids(std::vector<int32_t>& ids, const BPEPair& pair, int32_t idx) {
            int i =0;
            while (i<ids.size()) {
                if(i<ids.size()-1 && ids[i] == pair.first && ids[i+1] == pair.second) {
                    auto itr = ids.erase(ids.begin()+i, ids.begin()+i+1);
                    ids.insert(itr, idx);
                }
                i++;
            }
        }

        static BPEPair get_min_pair(const BPERanks& stats) {

        }

        static BPEPair get_max_pair(const BPERanks& stats) {
            int32_t max = INT32_MIN;
            auto max_pair_itr = stats.end();
            for (auto itr=stats.begin();itr!=stats.end();++itr) {
                if (itr->second>max) {
                    max = itr->second;
                    max_pair_itr = itr;
                }
            }
            return max_pair_itr->first;
        }

        static void compute_pairs_state(const std::vector<int32_t>& ids, BPERanks& stats) {
            for(int i=0;i<ids.size()-1;++i) {
                BPEPair pair {ids[i], ids[i+1]};
                if (stats.contains(pair)) {
                    stats[pair]++;
                } else {
                    stats.emplace(pair, 1);
                }
            }
        }

        static BPERanks compute_pairs_state(const std::vector<int32_t>& ids) {
            BPERanks stats;
            compute_pairs_state(ids, stats);
            return stats;
        }

        static std::string conv_to_utf8_string(const UnicodeString& unicode_string) {
            std::string buf;
            unicode_string.toUTF8String(buf);
            return buf;
        }

        static UnicodeString conv_to_unicode_string(const std::string& utf8_string) {
            return UnicodeString::fromUTF8(utf8_string);
        }


        static UnicodeString join_with_seperator(const UnicodeString& sep, const std::ranges::input_range auto& parts) {
            UnicodeString result;
            for(int i=0;const auto& part: parts) {
                result += part;
                if(++i < parts.size()) {
                    result += sep;
                }
            }
            return result;
        }

        template<int max_split_size=10>
        static void split_text_with_regex(const UnicodeString& text, const UnicodeString& seperator, std::vector<UnicodeString>& result) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher matcher(seperator, 0, status);
            UnicodeString parts[max_split_size];
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex with seperator string: " + seperator.toUTF8String(sep_utf8));
            }
            // we do exhaustive splitting using do-while loop
            int32_t splits_size = 0;
            do {
                splits_size = matcher.split(text, parts, max_split_size, status);
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to split text with seperator regex");
                }
                // validate if splits is all done
                matcher.reset(parts[splits_size-1]);
                if(matcher.find()) {
                    // insert all except last split
                    result.insert(result.end(), parts, parts+splits_size-2);
                    // do it recusively
                    splits_size = matcher.split(parts[splits_size-1], parts, max_split_size, status);
                }
            } while(max_split_size == splits_size);
            result.insert(result.end(), parts, parts+splits_size-1);
        }

        static std::vector<std::string> _bpe(const BPETokenRanks& bpe_token_ranks, const Bytes& token, int32_t max_rank) {
            auto part_view = token | std::views::transform([](char c) {
               return std::string({c});
            });
            auto parts = std::vector(part_view.begin(), part_view.end());
            while (true) {
                int min_rank = INT32_MAX;
                int min_id = -1;
                for(int i=0;i<parts.size()-1;++i) {
                    auto key = parts[i] + parts[i+1];
                    if (bpe_token_ranks.contains(key)) {
                        auto rank = bpe_token_ranks.at(key);
                        if(rank < min_rank) {
                            min_rank = rank;
                            min_id = i;
                        }
                    }
                }
                if(min_id == -1 || min_rank >= max_rank) {
                    // 1. found min_rank, but it's greater than max_rank
                    // 2. min_rank is not found (by checking min_id==-1)
                    break;
                }

                std::vector<std::string> new_parts;
                if(min_id > 0) {
                    for(int i=0;i<min_id;i++) {
                        new_parts.push_back(parts[i]);
                    }
                }
                new_parts.push_back(parts[min_id]);
                if(min_id+1 < parts.size()) {
                    new_parts.push_back(parts[min_id+1]);
                }
                if(min_id+2 < parts.size()) {
                    for(int i=min_id+2;i<parts.size();i++) {
                        new_parts.push_back(parts[i]);
                    }
                }
                parts = std::move(new_parts);
            }
            return parts;
        }

        static BPERanks recover_byte_pair_bpe_ranks(const BPETokenRanks& bpe_token_ranks) {
            BPERanks bpe_ranks;
            for(const auto&[token, rank]: bpe_token_ranks) {
                if(token.size()==1) {
                    continue;
                }
                auto pair = _bpe(bpe_token_ranks, token, rank);
                if (pair.size()!=2) {
                    throw InstinctException("failed to recover bit-level token for token string: " + token);
                }
                auto id0 = bpe_token_ranks.at(pair[0]);
                auto id1 = bpe_token_ranks.at(pair[1]);
                bpe_ranks.insert({std::pair{id0,id1}, rank});
            }
            return bpe_ranks;
        }
    }


}


#endif //TOKENIZER_HPP
