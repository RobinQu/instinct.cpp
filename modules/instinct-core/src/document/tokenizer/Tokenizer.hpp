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
#include "tools/TensorUtils.hpp"
#include <unicode/ustream.h>
#include <unicode/schriter.h>
#include <unicode/unimatch.h>
#include <tsl/ordered_map.h>


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


    /**
     * UTF-8 raw bytes, where single char range from -128 to 127. To make things easier, we use std::string directly as it has many conviniences comparing to `std::vector<uint8_t>`.  In BPE, token rank or token index is often stored in u_int8 which ranges from 0 to 255. So explict casting should be done to convert char, or int8_t to U32Char in ICU, or int32_t in BPEPair and BPERanks below.
     */
    using Bytes = std::string;
    using BPEPair = std::pair<int32_t, int32_t>;
    /**
     * many algorithms depends on maps with insertion-order like python's dict or OrderedDict. To make things easier, we try to do the same.
     *
     * FYI, in Python docs:
     * Ordered dictionaries are just like regular dictionaries but have some extra capabilities relating to ordering operations. They have become less important now that the built-in dict class gained the ability to remember insertion order (this new behavior became guaranteed in Python 3.7).
     *
     */
    using BPERanks = tsl::ordered_map<BPEPair, int32_t, hash_pair_int32_t>;
    using Vocab = tsl::ordered_map<int32_t, Bytes>;

    // for storing mappings special token to id
    using StringIDDict = std::unordered_map<UnicodeString, int32_t, hash_unicode_string>;
    using ReversedStringIDDict = std::unordered_map<int32_t, UnicodeString>;

    // using BPETokenPair = std::pair<UnicodeString, UnicodeString>;

    using BPETokenRanks = tsl::ordered_map<Bytes, int32_t>;




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

        // static RegexMatcher* create_regex_matcher(const std::string& pattern) {
        //     UErrorCode status = U_ZERO_ERROR;
        //     auto* regex_matcher = new RegexMatcher(UnicodeString::fromUTF8(pattern), 0, status);
        //     if(U_FAILURE(status)) {
        //         std::string sep_utf8;
        //         throw InstinctException("Failed to compile regex with pattern string: " + pattern);
        //     }
        //     return regex_matcher;
        // }

        static void merge_u32_ids(std::vector<int32_t>& ids, const BPEPair& pair, int32_t idx) {
            for(auto itr=ids.begin();itr!=ids.end();++itr) {
                if(*itr == pair.first && itr+1!=ids.end() && *(itr+1) == pair.second) {
                    itr = ids.erase(itr, itr+2);
                    itr = ids.insert(itr, idx);
                }
            }
        }

        static BPEPair get_min_pair(const BPERanks& stats) {
            int32_t min = INT32_MAX;
            auto min_pair_itr = stats.end();
            for (auto itr=stats.begin();itr!=stats.end();++itr) {
                if (itr->second<min) {
                    min = itr->second;
                    min_pair_itr = itr;
                }
            }
            return min_pair_itr->first;
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

        static void find_all_with_regex(const UnicodeString& text, const UnicodeString& regexp_pattern, std::vector<UnicodeString>& result) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher regex_matcher_(regexp_pattern, 0, status);
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex with seperator string: " + regexp_pattern.toUTF8String(sep_utf8));
            }
            regex_matcher_.reset(text);
            while (regex_matcher_.find()) {
                const int start = regex_matcher_.start(status);
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to match start during encoding");
                }
                int end = std::min(regex_matcher_.end(status), text.length());
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to match end during encoding");
                }
                result.push_back(text.tempSubStringBetween(start, end));
            }
        }

        /**
         *
         * https://stackoverflow.com/questions/39228912/stdregex-escape-special-characters-for-use-in-regex
         * @param s input string
         * @return escaped string
         */
        static UnicodeString escape_for_regular_expression(const UnicodeString& s) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher regex_matcher(R"(([\\\.\^\$\-\+\(\)\[\]\{\}\|\?\*]))", 0, status);
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex");
            }
            regex_matcher.reset(s);
            return regex_matcher.replaceAll(R"(\\$0)", status);
        }

        template<int max_split_size=10>
        static void split_text_with_regex(const UnicodeString& text, const UnicodeString& seperator, std::vector<UnicodeString>& result) {
            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher matcher(seperator, 0, status);
            if(U_FAILURE(status)) {
                std::string sep_utf8;
                throw InstinctException("Failed to compile regex with seperator string: " + seperator.toUTF8String(sep_utf8));
            }
            UnicodeString parts[max_split_size];
            // we do exhaustive splitting using do-while loop
            int32_t splits_size = 0;
            auto text_to_be_split = text;
            // std::cout << "input=" << text_to_be_split << std::endl;
            do {
                splits_size = matcher.split(text_to_be_split, parts, max_split_size, status);
                // for(int i=0;i<splits_size;i++) {
                //     std::cout << "len=" << parts[i].length() << ": "<<  parts[i] << std::endl;
                // }
                if(U_FAILURE(status)) {
                    throw InstinctException("Failed to split text with seperator regex");
                }
                // validate if splits is all done
                matcher.reset(parts[splits_size-1]);
                if(matcher.find()) {
                    // insert all except last split
                    result.insert(result.end(), parts, parts+splits_size-1);
                    // do it recusively
                    // splits_size = matcher.split(parts[splits_size-1], parts, max_split_size, status);
                    text_to_be_split = parts[splits_size-1];
                }
            } while(max_split_size == splits_size);

            result.insert(result.end(), parts, parts+splits_size);
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
                new_parts.push_back(parts[min_id] + parts[min_id+1]);
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
