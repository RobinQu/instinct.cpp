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

    using BPEPair = std::pair<int32_t, int32_t>;
    using BPERanks = std::unordered_map<BPEPair, int32_t>;
    using Bytes = std::string;
    using Vocab = std::unordered_map<int32_t, UnicodeString>;
    using RevseredVocab = std::unordered_map<UnicodeString, int32_t>;

    // using BPETokenPair = std::pair<UnicodeString, UnicodeString>;
    using BPETokenRanks = std::unordered_map<UnicodeString, int32_t>;


    class PretrainedTokenizer {
    public:
        PretrainedTokenizer()=delete;
        virtual ~PretrainedTokenizer()=default;
        PretrainedTokenizer(PretrainedTokenizer&&)=delete;
        PretrainedTokenizer(const PretrainedTokenizer&)=delete;
        virtual std::vector<int32_t> Encode(const std::string& text) = 0;
        virtual std::string Decode(const std::vector<int32_t>& ids) = 0;
        // virtual size_t GetVocabSize() = 0;
        // virtual std::string IdToToken(int32_t id) = 0;
        // virtual int32_t TokenToId(const std::string& token) = 0;
    };


    /**
     * following details are invisible to you
     */
    namespace details {

        static void merge_u32_ids(std::vector<int32_t>& ids, const BPEPair& pair, int32_t idx) {
            int i =0;
            while (i<ids.size()) {
                if(i<ids.size()-1 && ids[i] == pair.first && ids[i+1] == pair.second) {
                    auto itr = ids.erase(ids.begin()+i, ids.begin()+i+1);
                    ids.insert(itr, idx);
                }
            }

        }
        static BPERanks compute_pairs_state(const std::vector<int32_t>& ids) {
            BPERanks stats;
            for(int i=0;i<ids.size()-1;++i) {
                BPEPair pair {ids[i], ids[i+1]};
                if (stats.contains(pair)) {
                    stats[pair]++;
                } else {
                    stats.emplace(pair, 1);
                }
            }
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
                throw LangchainException("Failed to compile regex with seperator string: " + seperator.toUTF8String(sep_utf8));
            }
            // we do exhaustive splitting using do-while loop
            int32_t splits_size = 0;
            do {
                splits_size = matcher.split(text, parts, max_split_size, status);
                if(U_FAILURE(status)) {
                    throw LangchainException("Failed to split text with seperator regex");
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



        static BPERanks recover_byte_pair_bpe_ranks(const BPETokenRanks& bpe_token_ranks) {

        }




    }


}


#endif //TOKENIZER_HPP
