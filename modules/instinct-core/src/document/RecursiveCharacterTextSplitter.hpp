//
// Created by RobinQu on 2024/2/28.
//

#ifndef RECURSIVECHARACTERTEXTSPLITTER_HPP
#define RECURSIVECHARACTERTEXTSPLITTER_HPP
#include "CoreGlobals.hpp"
#include "TextSplitter.hpp"
#include <unicode/regex.h>
#include <unicode/ustring.h>
#include <unicode/brkiter.h>
#include "LanguageSplitters.hpp"
#include "tokenizer/TiktokenTokenizer.hpp"
#include "tokenizer/Tokenizer.hpp"

namespace INSTINCT_CORE_NS {

    using namespace U_ICU_NAMESPACE;

    using LengthFunction = std::function<int32_t(const UnicodeString&)> ;

    static LengthFunction IdentityLengthFunction = [](const UnicodeString& s)->int32_t {
        return s.length();
    };

    static std::vector<UnicodeString> DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER = {
        "\n\n",
        "\n",
        " ",
        ""
    };

    struct RecursiveCharacterTextSplitterOptions {
        LengthFunction length_function = IdentityLengthFunction;
        std::vector<UnicodeString> seperators = DEFAULT_SEPERATOR_FOR_TEXT_SPLITTER;
        int chunk_size=4000;
        int chunk_overlap=200;
        bool keep_sepeartor=false;
        bool strip_whitespace=true;
    };


    /**
     * \brief Recusively split text using a sequnce of given characters as splitter. Copied a lot from Langchain Python.
     */
    class RecursiveCharacterTextSplitter: public TextSplitter {
    public:
        RecursiveCharacterTextSplitter()=delete;
        explicit RecursiveCharacterTextSplitter(const RecursiveCharacterTextSplitterOptions& options)
            : chunk_size_(options.chunk_size),
              chunk_overlap_(options.chunk_overlap),
              keep_sepeartor_(options.keep_sepeartor),
              strip_whitespace_(options.strip_whitespace),
              length_function_(options.length_function),
              seperators_(options.seperators) {
        }

        static RecursiveCharacterTextSplitter* FromTiktokenTokenizer(TiktokenTokenizer* tokenizer, RecursiveCharacterTextSplitterOptions options = {}) {
            options.length_function = [=](const UnicodeString& str)->int32_t {
                return tokenizer->Encode(str).size();
            };
            return new RecursiveCharacterTextSplitter(options);
        }

        std::vector<std::string> SplitText(const std::string& text) override {
            auto seps = std::vector(seperators_);
            auto splits = SplitText_(UnicodeString::fromUTF8(text), seps);
            auto string_view = splits | std::views::transform(details::conv_to_utf8_string);
            return {string_view.begin(), string_view.end()};
        }
    private:
        std::vector<UnicodeString> SplitText_(const UnicodeString& text, std::vector<UnicodeString>& seperators) {

            // static auto empty = UnicodeString::fromUTF8("");
            std::vector<UnicodeString> final_chunks;

            // default to last sep, assuming it's most common case in text
            UnicodeString seperator = details::escape_for_regular_expression(seperators.back());
            for(auto itr=seperators.begin(); itr!=seperators.end(); ++itr) {
                auto sep = *itr;
                // break if it's empty string
                if(sep == "") {
                    seperator = details::escape_for_regular_expression(sep);
                    break;
                }
                // break if text can be spllited by sep
                if(text.indexOf(sep) > 0) {
                    seperator = details::escape_for_regular_expression(sep);
                    seperators = std::vector(itr+1, seperators.end());
                    break;
                }
            }

            auto splits = details::split_text_with_seperator(text, seperator, keep_sepeartor_);
            std::vector<UnicodeString> good_splits;
            for(auto& s: splits) {
                if(length_function_(s) < chunk_size_) {
                    good_splits.push_back(s);
                } else {
                    if(!good_splits.empty()) {
                        auto merged_text = MergeSplits_(good_splits, seperator);
                        final_chunks.insert(final_chunks.end(), merged_text.begin(), merged_text.end());
                        good_splits.clear();
                    }
                    if(seperators.empty()) {
                        final_chunks.push_back(s);
                    } else {
                        auto other = SplitText_(s, seperators);
                        final_chunks.insert(final_chunks.end(), other.begin(), other.end());
                    }
                }
            }

            if(!good_splits.empty()) {
                auto merged_text = MergeSplits_(good_splits, seperator);
                final_chunks.insert(final_chunks.end(), merged_text.begin(), merged_text.end());
            }
            return final_chunks;
        }

        [[nodiscard]] std::vector<UnicodeString> MergeSplits_(const std::vector<UnicodeString>& splits, const UnicodeString& seperator) const {
            const auto s_len = length_function_(seperator);
            std::vector<UnicodeString> docs;
            std::vector<UnicodeString> current_doc;
            int32_t total = 0;
            for(const auto& s: splits) {
                const auto d_len = length_function_(s);
                if(const auto len1 = total + d_len + (current_doc.empty() ? 0 : s_len); len1 > chunk_size_) {
                    if(total>chunk_size_) {
                        // TODO warns
                    }
                    if(!current_doc.empty()) {
                        if (const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                            docs.push_back(doc);
                        }
                        const auto len2 = total + d_len + (current_doc.empty() ? 0 : s_len);
                        while (total>chunk_overlap_ || len2>chunk_size_) {
                            total -= length_function_(current_doc[0]) + (current_doc.size()>1 ? s_len :0);
                            current_doc.erase(current_doc.begin());
                        }
                    }
                }
                current_doc.push_back(s);
                total += d_len + (current_doc.size()>1 ? s_len :0);
            }
            if(const auto doc = JoinDocs_(current_doc, seperator); doc.length()) {
                docs.push_back(doc);
            }
            return docs;
        }

        [[nodiscard]] UnicodeString JoinDocs_(const std::vector<UnicodeString>& docs, const UnicodeString& seperator) const  {
            UnicodeString text;
            for(int i=0; auto&& doc: docs) {
                text+=doc;
                if(++i < docs.size()) {
                    text+=seperator;
                }
            }
            if(strip_whitespace_) {
                return text.trim();
            }
            return text;
        }

        int chunk_size_;
        int chunk_overlap_;
        bool keep_sepeartor_;
        bool strip_whitespace_;
        LengthFunction length_function_;
        std::vector<UnicodeString> seperators_{};
    };
}

#endif //RECURSIVECHARACTERTEXTSPLITTER_HPP
