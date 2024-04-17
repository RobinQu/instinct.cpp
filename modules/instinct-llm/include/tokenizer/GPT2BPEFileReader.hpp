//
// Created by RobinQu on 2024/3/2.
//

#ifndef GPT2BPEFILEREADER_HPP
#define GPT2BPEFILEREADER_HPP


#include "BPETokenRanksReader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unicode/ustream.h>
#include "tools/StringUtils.hpp"
#include <nlohmann/json.hpp>
#include <unicode/schriter.h>



namespace INSTINCT_LLM_NS {
    using namespace INSTINCT_CORE_NS;
    class GPT2BPEFileReader : public BPETokenRanksReader {
        std::filesystem::path vocab_bpe_file_path_;
        std::filesystem::path encoder_json_file_path_;
        std::unique_ptr<BPETokenRanks> cache_;

    public:
        GPT2BPEFileReader(std::filesystem::path vocab_bpe_file_path, std::filesystem::path encoder_json_file_path)
            : vocab_bpe_file_path_(std::move(vocab_bpe_file_path)),
              encoder_json_file_path_(std::move(encoder_json_file_path)) {
        }

        BPETokenRanks Fetch() override {
            if(!cache_) {
                cache_ = std::make_unique<BPETokenRanks>();
                DoFetch_(*cache_);
            }
            return *cache_;
        }

    private:

        void DoFetch_(BPETokenRanks& bpe_token_ranks) {
            std::ifstream vocab_bpe_file(vocab_bpe_file_path_);

            std::vector<u_int8_t> rank_to_byte;
            // for (int i = 0; i < 256; i++) {
            //     if (u_isprint(i) && UnicodeString(i) != ' ') {
            //         rank_to_byte.push_back(i);
            //     }
            // }

            // NOTE: sadly, u_isprint(i) is not equivalent to str.isprintable() in Python. Codepoint 166 in unicode is an example when `u_isprint` in C++ returns true but `str.isprintable` return false in Python.
            auto insert_fn = [&](u_int32_t a, u_int32_t b) {
                for(u_int32_t begin = a; begin<=b; begin++) {
                    rank_to_byte.push_back(begin);
                }
            };
            insert_fn(L'!', L'~');
            insert_fn(L'¡', L'¬');
            insert_fn(L'®', L'ÿ');

            tsl::ordered_map<UChar32, u_int8_t> data_gym_byte_to_byte;
            for (const auto& i: rank_to_byte) {
                data_gym_byte_to_byte[static_cast<UChar32>(i)] = i;
            }

            size_t n = 0;
            for (int i = 0; i < 256; i++) {
                bool found = false;
                for (const auto& rank: rank_to_byte) {
                    if (i == rank) {
                        found = true;
                    }
                }
                if (!found) {
                    rank_to_byte.push_back(i);
                    data_gym_byte_to_byte[static_cast<UChar32>(256 + n)] = i;
                    n += 1;
                }
            }

            if (!vocab_bpe_file.is_open()) {
                throw InstinctException("failed to open vocab file: " + vocab_bpe_file_path_.string());
            }


            std::vector<UnicodeString> lines;
            for (std::string line; std::getline(vocab_bpe_file, line);) {
                lines.push_back(UnicodeString::fromUTF8(line));
            }


            std::vector<std::pair<UnicodeString, UnicodeString>> bpe_merges;
            for (size_t i=0,len=lines.size();i<len;++i) {
                // last new line is ommitted already by std::get_line
                if (i == 0) {
                    continue;
                }
                auto l = lines[i];
                std::vector<UnicodeString> splits;
                U32StringUtils::SpilitWithRegex(l, UnicodeString::fromUTF8("\\s"), splits);
                if (splits.size() != 2) {
                    //TODO should warn about invalid line
                    continue;
                }
                bpe_merges.emplace_back(splits[0], splits[1]);
            }


            auto decode_data_gym = [&](const UnicodeString& str) {
                std::string buf;

                StringCharacterIterator itr(str);
                while (itr.hasNext()) {
                    auto c = itr.next32PostInc();
                    if (data_gym_byte_to_byte.contains(c)) {
                        buf += static_cast<char>(data_gym_byte_to_byte.at(c));
                    } else {
                        throw InstinctException("char not found " + std::to_string(c));
                    }
                }
                return buf;
            };

//            BPETokenRanks bpe_token_ranks;
            for (int i = 0; i < rank_to_byte.size(); i++) {
                const auto key = Bytes({static_cast<char>(rank_to_byte[i])});
                bpe_token_ranks[key] = i;
            }
            n = bpe_token_ranks.size();
            for (const auto& [first,second]: bpe_merges) {
                auto key = decode_data_gym(first) + decode_data_gym(second);
                bpe_token_ranks[key] = n;
                n += 1;
            }


            // validate against encoder json
            std::ifstream encoder_json_file(encoder_json_file_path_);
            auto encode_json = nlohmann::json::parse(encoder_json_file);
            BPETokenRanks encoder_json_loaded;
            for (auto& el: encode_json.items()) {
                // UnicodeString key = UnicodeString::fromUTF8(el.key());
                // encoder_json_loaded.emplace(decode_data_gym(key), el.value().get<int32_t>());
                auto key = decode_data_gym(UnicodeString::fromUTF8(el.key()));
                if (encoder_json_loaded.contains(key)) {
                    throw InstinctException("internal data error due to duplicated token: " + key);
                }
                encoder_json_loaded.emplace(key, el.value().get<int32_t>());
            }
            encoder_json_loaded.erase("<|endoftext|>");
            encoder_json_loaded.erase("<|startoftext|>");
            for (const auto& key: encoder_json_loaded | std::views::keys) {
                if (!bpe_token_ranks.contains(key)) {
                    throw InstinctException(
                            "invalid vocab bpe or encoder json file, as key found in encoder json but not in vocab bpe: " +
                            key);
                }
            }
            for (const auto& key: bpe_token_ranks | std::views::keys) {
                if (!encoder_json_loaded.contains(key)) {
                    throw InstinctException(
                            "invalid vocab bpe or encoder json file, as key found in vocab bpe but not in encoder json: " +
                            key);
                }
            }
//            return bpe_token_ranks;
        }
    };


}

#endif //GPT2BPEFILEREADER_HPP
