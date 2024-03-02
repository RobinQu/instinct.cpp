//
// Created by RobinQu on 2024/3/2.
//

#ifndef GPT2BPEFILEREADER_HPP
#define GPT2BPEFILEREADER_HPP


#include "BPEFileReader.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unicode/ustream.h>
#include "tools/StringUtils.hpp"
#include <nlohmann/json.hpp>
#include <unicode/schriter.h>



namespace INSTINCT_CORE_NS {
    class GPT2BPEFileReader : public BPEFileReader {
        std::filesystem::path vocab_bpe_file_path_;
        std::filesystem::path encoder_json_file_path_;

    public:
        GPT2BPEFileReader(std::filesystem::path vocab_bpe_file_path, std::filesystem::path encoder_json_file_path)
            : vocab_bpe_file_path_(std::move(vocab_bpe_file_path)),
              encoder_json_file_path_(std::move(encoder_json_file_path)) {
        }

        BPETokenRanks Fetch() override;
    };

    inline BPETokenRanks GPT2BPEFileReader::Fetch() {
        std::ifstream vocab_bpe_file(vocab_bpe_file_path_);

        std::vector<u_int8_t> rank_to_byte;
        for (int i = 0; i < 256; i++) {
            if (u_isprint(i) && UnicodeString(i) != ' ') {
                rank_to_byte.push_back(i);
            }
        }

        std::unordered_map<UChar32, u_int8_t> data_gym_byte_to_byte;
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
            throw LangchainException("failed to open vocab file: " + vocab_bpe_file_path_.string());
        }


        std::vector<UnicodeString> lines;

        UnicodeString line;
        while (vocab_bpe_file) {
            vocab_bpe_file >> line;
            lines.push_back(line);
        }

        std::vector<std::pair<UnicodeString, UnicodeString>> bpe_merges;
        for (size_t i=0,len=lines.size();i<len;++i) {
            if (i == 0 || i == len - 1) {
                continue;
            }
            auto l = lines[i];
            std::vector<UnicodeString> splits;
            details::split_text_with_regex(l, UnicodeString::fromUTF8("\n"), splits);
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
                buf.append({static_cast<char>(data_gym_byte_to_byte[c])});
            }
            return buf;
        };

        BPETokenRanks bpe_token_ranks;
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
            encoder_json_loaded.emplace(el.key(), el.value().get<int32_t>());
        }
        encoder_json_loaded.erase("<|endoftext|>");
        encoder_json_loaded.erase("<|startoftext|>");
        for (const auto& key: encoder_json_loaded | std::views::keys) {
            if (!bpe_token_ranks.contains(key)) {
                throw LangchainException(
                    "invalid vocab bpe or encoder json file, as key found in encoder json but not in vocab bpe: " +
                    key);
            }
        }
        for (const auto& key: bpe_token_ranks | std::views::keys) {
            if (!encode_json.contains(key)) {
                throw LangchainException(
                    "invalid vocab bpe or encoder json file, as key found in vocab bpe but not in encoder json: " +
                    key);
            }
        }
        return bpe_token_ranks;
    }
}

#endif //GPT2BPEFILEREADER_HPP
